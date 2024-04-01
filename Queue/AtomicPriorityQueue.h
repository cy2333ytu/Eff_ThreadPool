#ifndef ATOMIC_PRIORITY_QUEUE
#define ATOMIC_PRIORITY_QUEUE

#include "QueueObject.h"
#include <queue>
#include <vector>
#include <memory>
namespace ccy
{

template<typename T>
class AtomicPriorityQueue: public QueueObject{
    public:
        AtomicPriorityQueue() = default;
    /**
     * 尝试弹出
     * @param value
     * @return
     */
    bool tryPop(T& value){
        bool result = false;
        if(mutex_.try_lock()){
            if(!priority_queue_.empty()){
                value = std::move(*priority_queue_.top());
                priority_queue_.pop();
                result = true;
            }
            mutex_.unlock();
        }
        return result;
    }

    /**
     * 尝试弹出多个任务,只要有一个元素被弹出，则return true
     * @param values
     * @param maxPoolBatchSize
     * @return
     */

    bool tryPop(std::vector<T>& values, int maxPoolBatchSize){
        bool result = false;
        if(mutex_.try_lock()){
            while(!priority_queue_.empty() && maxPoolBatchSize-- > 0){
                values.emplace_back(std::move(*priority_queue_.top()));
                priority_queue_.pop();
                result = true;
            }
            mutex_.unlock();
        }
        return result;
    }

    /**
     * 传入数据
     * @param value
     * @param priority 任务优先级，数字排序
     * @return
     */
    void push(T&& value, int priority){
        std::unique_ptr<T> task(c_make_unique<T>(std::move(value), priority));
        LOCK_GUARD lk(mutex_);
        priority_queue_.push(std::move(task));
    }
     
    /**
     * 判定队列是否为空
     * @return
     */
    bool empty() {
        LOCK_GUARD lk(mutex_);
        return priority_queue_.empty();
    }

    NO_ALLOWED_COPY(AtomicPriorityQueue)                

    private:
        std::priority_queue<std::unique_ptr<T>> priority_queue_;
};

}

#endif