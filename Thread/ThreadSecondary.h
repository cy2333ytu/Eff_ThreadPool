#ifndef THREADSECONDARY_H
#define THREADSECONDARY_H

#include "ThreadBase.h"

namespace ccy
{

class ThreadSecondary: public ThreadBase{
public:
    explicit ThreadSecondary(){
        cur_ttl_ = 0;
        type_ = THREAD_TYPE_SECONDARY;
    }

protected:
    Status init() override{
        Status status;
        ASSERT_INIT(false)
        ASSERT_NOT_NULL(config_)
        
        cur_ttl_ = config_->secondary_thread_ttl_;
        is_init_ = true;
        thread_ = std::move(std::thread(&ThreadSecondary::run(), this));
        setSchedParam();
        return status;
    }

    /**
     * 设置pool的信息
     * @param poolTaskQueue
     * @param poolPriorityTaskQueue
     * @param config
     * @return
     */
    Status setThreadPoolInfo(AtomicQueue<Task>* poolTaskQueue,
                              AtomicPriorityQueue<Task>* poolPriorityTaskQueue,
                              ThreadPoolConfigPtr config)
            {
                Status status;
                ASSERT_INIT(false)
                ASSERT_NOT_NULL(poolTaskQueue, poolPriorityTaskQueue, config)

                this->pool_task_queue_ = poolTaskQueue;
                this->pool_priority_task_queue_ = poolPriorityTaskQueue;
                this->config_ = config;
                return status;
            }

    Status run() override{
            Status status;
            ASSERT_INIT(true)

            status = loopProcess();
            return status;
    }

    void processTask() override {
        Task task;
        if (popPoolTask(task)) {
            runTask(task);
        } else {
            // 如果任务无法获取，则稍加等待
            waitRunTask(config_->queue_emtpy_interval_);
        }
    }

    /**
     * 有等待的执行任务
     * @param ms
     * @return
     * @notice 目的是降低cpu的占用率
     */
    void waitRunTask(long ms) {
        auto task = this->pool_task_queue_->popWithTimeout(ms);
        if (nullptr != task) {
            (*task)();
        }
    }
    void processTasks() override {
        TaskArr tasks;
        if (popPoolTask(tasks)) {
            runTasks(tasks);
        } else {
            waitRunTask(config_->queue_emtpy_interval_);
        }
    }
    
    /**
     * 判断本线程是否需要被自动释放
     * @return
     */
    bool freeze(){
        if(likely(is_running_)){
            cur_ttl_++;
            cur_ttl_ = std::min(cur_ttl_, config_->secondary_thread_ttl_);
        }else{
            cur_ttl_--;                                         // 如果当前线程没有在执行，则ttl-1
        }
        return cur_ttl_ <=0 && done_;                           // 必须是正在执行的线程，才可以被回收
    }

private:
    int cur_ttl_ = 0;                                              // 当前最大生存周期

    friend class UThreadPool;
};

using ThreadSecondaryPtr = ThreadSecondary *;

}


#endif