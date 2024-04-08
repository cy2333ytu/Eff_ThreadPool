#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "ThreadObject.h"
#include "ThreadPoolConfig.h"
#include "Queue/QueueInclude.h"
#include "Thread/ThreadInclude.h"
#include "Task/TaskInclude.h"

#include <vector>
#include <list>
#include <map>
#include <future>
#include <thread>
#include <algorithm>
#include <memory>
#include <functional>

namespace ccy
{ 
    
class ThreadPool : public ThreadObject {
public:
    /**
     * 通过默认设置参数，来创建线程池
     * @param autoInit 是否自动开启线程池功能
     * @param config
     */
    explicit ThreadPool(bool autoInit = true,
                    const ThreadPoolConfig& config = ThreadPoolConfig()) noexcept;

    /**
     * 析构函数
     */
    ~ThreadPool() override;

    /**
     * 设置线程池相关配置信息
     * @param config
     * @return
     */
    Status setConfig(const ThreadPoolConfig& config);

    /**
     * 开启所有的线程信息
     * @return
     */
    Status init() final;

    /**
     * 提交任务信息
     * @tparam FunctionType
     * @param func
     * @param index
     * @return
     */
    template<typename FunctionType>
    auto commit(const FunctionType& func, int index = DEFAULT_TASK_STRATEGY)
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


    /**
     * 根据优先级，执行任务
     * @tparam FunctionType
     * @param func
     * @param priority 优先级别。自然序从大到小依次执行
     * @return
     * @notice priority 范围在 [-100, 100] 之间
     */
    template<typename FunctionType>
    auto commitWithPriority(const FunctionType& func, int priority)
        -> std::future<decltype(std::declval<FunctionType>()())>
    {
        using ResultType = decltype(std::declval<FunctionType>()());

        std::packaged_task<ResultType()> task(func);
        std::future<ResultType> result(task.get_future());

        if(secondary_threads_.empty()){
            createSecondaryThread(1);
        }
        priority_task_queue_(std::move(task), priority);
        
        return result;
    }

    /**
     * 执行任务组信息
     * 取taskGroup内部ttl和入参ttl的最小值，为计算ttl标准
     * @param taskGroup
     * @param ttl
     * @return
     */
    Status submit(const TaskGroup& taskGroup,
                   long ttl = MAX_BLOCK_TTL);

    /**
     * 针对单个任务的情况，复用任务组信息，实现单个任务直接执行
     * @param task
     * @param ttl
     * @param onFinished
     * @return
     */
    Status submit(DEFAULT_CONST_FUNCTION_REF func,
                   long ttl = MAX_BLOCK_TTL,
                   CALLBACK_CONST_FUNCTION_REF onFinished = nullptr);

    /**
     * 获取根据线程id信息，获取线程index信息
     * @param tid
     * @return
     * @notice 辅助线程返回-1
     */
    int getThreadIndex(size_t tid);
        /**
     * 释放所有的线程信息
     * @return
     */

    Status destroy() final;

    /**
     * 判断线程池是否已经初始化了
     * @return
     */
    bool isInit() const;

    /**
     * 生成辅助线程。内部确保辅助线程数量不超过设定参数
     * @param size
     * @return
     */
    Status createSecondaryThread(int size);

    /**
     * 删除辅助线程
     * @param size
     * @return
     */
    Status releaseSecondaryThread(int size);

protected:
    /**
     * 根据传入的策略信息，确定最终执行方式
     * @param origIndex
     * @return
     */
    virtual int dispatch(int origIndex);

    /**
     * 监控线程执行函数，主要是判断是否需要增加线程，或销毁线程
     * 增/删 操作，仅针对secondary类型线程生效
     */
    void monitor();

    NO_ALLOWED_COPY(ThreadPool)

private:
    bool is_init_ { false };                                                       // 是否初始化
    int cur_index_ = 0;                                                            // 记录放入的线程数
    AtomicQueue<Task> task_queue_;                                                // 用于存放普通任务
    AtomicPriorityQueue<Task> priority_task_queue_;                               // 运行时间较长的任务队列，仅在辅助线程中执行
    std::vector<ThreadPrimaryPtr> primary_threads_;                                // 记录所有的主线程
    std::list<std::unique_ptr<ThreadSecondary>> secondary_threads_;                // 记录所有的辅助线程
    ThreadPoolConfig config_;                                                      // 线程池的设置参数
    std::thread monitor_thread_;                                                    // 监控线程
    std::map<size_t, int> thread_record_map_;                                        // 线程记录的信息
    std::mutex st_mutex_;                                                           // 辅助线程发生变动的时候，加的mutex信息
};

using ThreadPoolPtr = ThreadPool *;
 
} // namespace ccy
 
#endif