#include "ThreadPool.h"
#include "./Utils/UtilsDefine.h"
#include "Allocator.h"
#include <vector>

namespace ccy
{

ThreadPool::ThreadPool(bool autoInit, const ThreadPoolConfig& config) noexcept
    {
        cur_index_ = 0;
        is_init_ = false;
        this->setConfig(config);
        if(autoInit){
            this->init();
        }
    }

ThreadPool::~ThreadPool()
    {
        this->config_.monitor_enable_ = false;
        if(monitor_thread_.joinable()){
            monitor_thread_.join();
        }
        destroy();
    }

Status ThreadPool::setConfig(const ThreadPoolConfig &config) {
    Status status;
    ASSERT_INIT(false)    // 初始化后，无法设置参数信息

    this->config_ = config;
    return status;
}

Status ThreadPool::init(){
    Status status;
    if(is_init_){
        return status;
    }
    monitor_thread_ = std::move(std::thread(&ThreadPool::monitor, this));
    thread_record_map_.clear();
    primary_threads_.reserve(config_.default_thread_size_);
    for(int i = 0; i < config_.default_thread_size_; i++){
        auto ptr = SAFE_MALLOC_OBJECT(ThreadPrimary);
        ptr->setThreadPoolInfo(i, &task_queue_, &primary_threads_, &config_);
    
        // 记录线程和匹配id信息
        thread_record_map_[(size_t)std::hash<std::thread::id>{}(ptr->thread_.get_id())] = i;
        primary_threads_.emplace_back(ptr);
    }

    for (auto* pt : primary_threads_) {
        status += pt->init();
    }

    FUNCTION_CHECK_STATUS
    status = createSecondaryThread(config_.secondary_thread_size_);
    FUNCTION_CHECK_STATUS

    is_init_ = true;
    return status;
}

Status ThreadPool::submit(const TaskGroup& taskGroup, long ttl){
    Status status;
    ASSERT_INIT(true)

    std::vector<std::future<void>> futures;
    for(const auto& task: taskGroup.task_arr_){
        futures.emplace_back(commit(task));
    }
    // 计算运行时间
    auto deadline = std::chrono::steady_clock::now()
            + std::chrono::milliseconds(std::min(taskGroup.getTtl(), ttl));

    for(auto& fut: futures){
        const auto& futStatus = fut.wait_until(deadline);
        switch (futStatus)
        {
            case std::future_status::ready: break;     // 正常情况，返回
            case std::future_status::timeout: status += ErrStatus("thread status timeout"); break;  
            case std::future_status::deferred: status += ErrStatus("thread status deferred"); break; 

            default: status += ErrStatus("thread status unknown");
        }
    }

    if(taskGroup.on_finished_){
        taskGroup.on_finished_(status);
    }
    return status;
}

Status ThreadPool::submit(DEFAULT_CONST_FUNCTION_REF func, long ttl,
                   CALLBACK_CONST_FUNCTION_REF onFinished)
            {
                return submit(TaskGroup(func, ttl, onFinished));
            }

int ThreadPool::getThreadIndex(size_t tid){
    int threadNum = SECONDARY_THREAD_COMMON_ID;
    auto result = thread_record_map_.find(tid);
    if(result != thread_record_map_.end()){
        threadNum = result->second;
    }
    return threadNum;
}

Status ThreadPool::destroy(){
    Status status;
    if(!is_init_){
        return status;
    }
    // delete primary
    for(auto &pt : primary_threads_){
        status += pt->destroy();
    }
    FUNCTION_CHECK_STATUS
    
    for (auto &pt : primary_threads_) {
        DELETE_PTR(pt)
    }
    primary_threads_.clear();

    // secondary is intel
    for(auto &st: secondary_threads_){
        status += st->destroy();
    }
    FUNCTION_CHECK_STATUS
    secondary_threads_.clear();
    thread_record_map_.clear();
    is_init_ = false;

    return status;
}

bool ThreadPool::isInit() const{
    return is_init_;
}

Status ThreadPool::releaseSecondaryThread(int size){
    Status status;
    LOCK_GUARD lock(st_mutex_);
    // 将所有已经结束的，删掉
    for(auto iter = secondary_threads_.begin(); iter != secondary_threads_.end();){
        !(*iter)->done_? secondary_threads_.erase(iter++) : iter++;
    }
    RETURN_ERROR_STATUS_BY_CONDITION((size > secondary_threads_.size()), \
                "cannot release [" + std::to_string(size) + "] secondary thread,"    \
                + "only [" + std::to_string(secondary_threads_.size()) + "] left.")

    // 标记需要删除的信息
    for(auto iter = secondary_threads_.begin(); iter != secondary_threads_.end() && size-- > 0;)
    {
        (*iter)->done_ = false;
        iter++;
    }
    return status;
}

int ThreadPool::dispatch(int origIndex){
    int realIndex = 0;
    if(DEFAULT_TASK_STRATEGY == origIndex){
        /**
         * 如果是默认策略信息，在[0, default_thread_size_) 之间的，通过 thread 中queue来调度
         * 在[default_thread_size_, max_thread_size_) 之间的，通过 pool 中的queue来调度
         */
        realIndex = cur_index_++;
        if(cur_index_ >= config_.max_thread_size_ || cur_index_ < 0){
            cur_index_ = 0;
        }
    }else{
        realIndex = origIndex;
    }
    return realIndex;         // 交到上游去判断，走哪个线程
}

Status ThreadPool::createSecondaryThread(int size){
    Status status;
    int leftSize = (int)(config_.max_thread_size_- config_.default_thread_size_ - secondary_threads_.size());
    int realSize = std::min(size, leftSize);

    LOCK_GUARD lock(st_mutex_);
    for(int i = 0; i < realSize; i++){
        auto ptr = MAKE_UNIQUE_OBJECT(ThreadSecondary)
        ptr->setThreadPoolInfo(&task_queue_, &priority_task_queue_, &config_);
        status += ptr->init();
        secondary_threads_.emplace_back(std::move(ptr));
    }

    return status;
}

void ThreadPool::monitor(){
    while(config_.monitor_enable_){
        while(config_.monitor_enable_ && !is_init_){
            // 如果没有init，则一直处于空跑状态
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        auto span = config_.monitor_span_;
        while(config_.monitor_enable_ && is_init_ && span--){
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // 若 primary线程都在执行，则表示忙碌
        bool busy = !primary_threads_.empty() && std::all_of(primary_threads_.begin(), primary_threads_.end(),
                                [](ThreadPrimaryPtr ptr) { return nullptr != ptr && ptr->is_running_; });

        LOCK_GUARD lock(st_mutex_);
        if(busy || !priority_task_queue_.empty()){
            createSecondaryThread(1);
        }
        
        // 判断 secondary 线程是否需要退出
        for (auto iter = secondary_threads_.begin(); iter != secondary_threads_.end(); ) {
            (*iter)->freeze() ? secondary_threads_.erase(iter++) : iter++;
        }
    }
}

}