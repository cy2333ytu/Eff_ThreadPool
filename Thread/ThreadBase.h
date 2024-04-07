#ifndef THREADBASE_H
#define THREADBASE_H


#include "../ThreadObject.h"
#include "../Queue/QueueInclude.h"
#include "../Task/TaskInclude.h"
#include "../ThreadPoolConfig.h"
#include <thread>
#include <iostream>

namespace ccy
{

class ThreadBase: public ThreadObject{
protected:
    explicit ThreadBase(){
        done_ = true;
        is_init_ = false;
        is_running_ = false;
        total_task_num_ = 0;
        pool_task_queue_ = nullptr;
        pool_priority_task_queue_ = nullptr;
        config_ = nullptr;
    }

    ~ThreadBase() override{
        reset();
    }

    /**
     * 所有线程类的 destroy 一样
     * @return
     */
    Status destroy() override {
        Status status;
        ASSERT_INIT(true)

        reset();
        return status;
    }

    /**
     * 从线程池的队列中，获取任务
     * @param task
     * @return
     */
    virtual bool popPoolTask(TaskRef task){
        bool result = pool_task_queue_->tryPop(task);
        if(!result && THREAD_TYPE_SECONDARY == type_){ 
            // 若辅助线程没有获取到的话，再尝试从任务队列中获取一次
            result = pool_priority_task_queue_->tryPop(task);
        }
        return result;
    }

    /**
     * 从线程池的队列中中，获取批量任务
     * @param tasks
     * @return
     */
    virtual bool popPoolTask(TaskArrRef tasks){
        bool result = pool_task_queue_->tryPop(tasks, config_->max_pool_batch_size_);
        if (!result && THREAD_TYPE_SECONDARY == type_) {
            result = pool_priority_task_queue_->tryPop(tasks, 1);    // 从优先队列里，pop出来一个
        }
        return result;
    }

    /**
     * 执行单个任务
     * @param task
     */
    void runTask(Task& task){
        is_running_ = true;
        task();
        total_task_num_++;
        is_running_ = false;
    }

    /**
     * 批量执行任务
     * @param tasks
     */
    void runTasks(TaskArr& tasks) {
        is_running_ = true;
        for (auto& task : tasks) {
            task();
        }
        total_task_num_ += tasks.size();
        is_running_ = false;
    }

    /**
     * 清空所有任务内容
     */
    void reset(){
        done_ = false;
        if(thread_.joinable()){
            thread_.join();
        }
        is_init_ = false;
        is_running_ = false;
        total_task_num_ = 0;
    }

    /**
     * 获取单个消息
     * @return
     */
    virtual void processTask() = 0;

    /**
     * 获取批量执行task信息
     */
    virtual void processTasks() = 0;

    /**
     * 循环处理任务
     * @return
     */

    Status loopProcess(){
        Status status;
        ASSERT_NOT_NULL(config_)
        if(config_->batch_task_enable_){
            while (done_){
                processTasks();                         // 执行批量任务
            }
        }else{
            while(done_){
                processTask();                          // 执行单个任务
            }
        }

        return status;
    }

    /**
    * 设置线程优先级
    */
    void setSchedParam() {
        int priority = THREAD_SCHED_OTHER;
        int policy = THREAD_MIN_PRIORITY;
        if (type_ == THREAD_TYPE_PRIMARY) {
            priority = config_->primary_thread_priority_;
            policy = config_->primary_thread_policy_;
        } else if (type_ == THREAD_TYPE_SECONDARY) {
            priority = config_->secondary_thread_priority_;
            policy = config_->secondary_thread_policy_;
        }

        auto handle = thread_.native_handle();
        sched_param param = { calcPriority(priority) };
        int ret = pthread_setschedparam(handle, calcPolicy(policy), &param);
        if (0 != ret) {
            std::cout << "warning : set thread sched param failed, system error code is " << ret;
        }
    }
    
    /**
     * 设定线程优先级信息
     * 超过[min,max]范围，统一设置为min值
     * @param priority
     * @return
     */
    static int calcPriority(int priority) {
        return (priority >= THREAD_MIN_PRIORITY
                && priority <= THREAD_MAX_PRIORITY)
               ? priority : THREAD_MIN_PRIORITY;
    }

    /**
     * 设定计算线程调度策略信息，
     * 非OTHER/RR/FIFO对应数值，统一返回OTHER类型
     * @param policy
     * @return
     */
    static int calcPolicy(int policy) {
        return (THREAD_SCHED_OTHER == policy
                || THREAD_SCHED_RR == policy
                || THREAD_SCHED_FIFO == policy)
               ? policy : THREAD_SCHED_OTHER;
    }
protected:
    bool done_;                                                        // 线程状态标记
    bool is_init_;                                                     // 标记初始化状态
    bool is_running_;                                                  // 是否正在执行
    int type_ = 0;                                                     // 用于区分线程类型（主线程、辅助线程）
    unsigned long total_task_num_ = 0;                                 // 处理的任务的数量

    AtomicQueue<Task>* pool_task_queue_;                             // 用于存放线程池中的普通任务
    AtomicPriorityQueue<Task>* pool_priority_task_queue_;            // 用于存放线程池中的包含优先级任务的队列，仅辅助线程可以执行
    ThreadPoolConfigPtr config_ = nullptr;                            // 配置参数信息
    std::thread thread_;                                               // 线程类


};

}

#endif