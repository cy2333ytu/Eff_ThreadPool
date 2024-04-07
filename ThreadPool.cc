#include "ThreadPool.h"
#include "./Utils/UtilsDefine.h"
#include "Allocator.h"

namespace ccy
{

ThreadPool::ThreadPool(bool autoInit = true, const ThreadPoolConfig& config) noexcept
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

template<typename FunctionType>
auto ThreadPool::commit(const FunctionType& func, int index)
    -> std::future<decltype(std::declval<FunctionType>()())>
    {
        using RetType = decltype(std::declval<FunctionType>()());

        std::packaged_task<RetType()> task(func);
        std::future<RetType> result(task.get_future());

        int realIndex = dispatch(index);
        if(realIndex >= 0 && realIndex < config_.default_thread_size_){
            // 如果返回的结果，在主线程数量之间，则放到主线程的queue中执行
            primary_threads_[realIndex]->pushTask(std::move(task));
        }else if(LONG_TIME_TASK_STRATEGY == realIndex){
            /**
             * 如果是长时间任务，则交给特定的任务队列，仅由辅助线程处理
             * 目的是防止有很多长时间任务，将所有运行的线程均阻塞
             * 长任务程序，默认优先级较低
             **/
            priority_task_queue_.push(std::move(task), LONG_TIME_TASK_STRATEGY);
        }else{
            task_queue_.push(std::move(task));
        }
        return result;
    }

template<typename FunctionType>
auto ThreadPool::commitWithPriority(const FunctionType& func, int priority)
    -> std::future<decltype(std::declval<FunctionType>()())>
    {
        using ResultType = decltype(std::declval<FunctionType>()());

        std::packaged_task<ResultType()> task(func);
        std::future<ResultType> result(task.get_future());

        if(secondary_threads_.empty()){
            createSecondaryThread(1);
        }
        priority_task_queue_(Std::move(task), priority);
        
        return result;
    }




}