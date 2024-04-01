#ifndef WORKSTEALINGQUEUE_H
#define WORKSTEALINGQUEUE_H

#include "QueueObject.h"
#include <vector>
#include <deque>
namespace ccy
{

template<typename T>
class WorkStealingQueue: public QueueObject{
    public:
        /**
         * 向队列中写入信息
         * @param task
         */
        void push(T&& task){
            while(true){
                if(lock_.try_lock()){
                    deque_.emplace_front(std::forward<T>(task));
                    lock_.unlock();
                    break;
                }else{
                    std::this_thread::yield();
                }
            }
        }

        /**
         * 尝试往队列里写入信息
         * @param task
         * @return
         */
        bool tryPush(T&& task){
            bool result = false;
            if(lock_.try_lock()){
                deque_.emplace_back(std::forward<T>(task));
                lock_.unlock();
                result = true;
            }
            return result;
        }

        /**
         * 向队列中写入信息
         * @param task
         */
        void push(std::vector<T>& tasks){
            while(true){
                if(lock_.try_lock()){
                    for(const auto& task: tasks){
                        deque_.emplace_front(std::forward<T>(task));
                    }
                    lock_.unlock();
                    break;
                }else{
                    std::this_thread::yield();
                }
            }
        }
    
        /**
         * 尝试批量写入内容
         * @param tasks
         * @return
         */

        bool tryPush(std::vector<T>& tasks) {
            bool result = false;
            if (lock_.try_lock()) {
                for (const auto& task : tasks) {
                    deque_.emplace_back(std::forward<T>(task));
                }
                lock_.unlock();
                result = true;
            }
            return result;
        }
            
        /**
         * 弹出节点，从头部进行
         * @param task
         * @return
         */
        bool tryPop(T& task) {
            bool result = false;
            if (!deque_.empty() && lock_.try_lock()) {
                if (!deque_.empty()) {
                    task = std::forward<T>(deque_.front());    // 从前方弹出
                    deque_.pop_front();
                    result = true;
                }
                lock_.unlock();
            }

            return result;
        }

        /**
         * 从头部开始批量获取可执行任务信息
         * @param taskArr
         * @param maxLocalBatchSize
         * @return
         */
        bool tryPop(std::vector<T>& taskArr, int maxLocalBatchSize) {
            bool result = false;
            if (!deque_.empty() && lock_.try_lock()) {
                while (!deque_.empty() && maxLocalBatchSize--) {
                    taskArr.emplace_back(std::forward<T>(deque_.front()));
                    deque_.pop_front();
                    result = true;
                }
                lock_.unlock();
            }

            return result;
        }
        /**
         * 窃取节点，从尾部进行
         * @param task
         * @return
         */
        bool trySteal(T& task) {
            bool result = false;
            if (!deque_.empty() && lock_.try_lock()) {
                if (!deque_.empty()) {
                    task = std::forward<T>(deque_.back());    // 从后方窃取
                    deque_.pop_back();
                    result = true;
                }
                lock_.unlock();
            }

            return result;
        }

        /**
         * 批量窃取节点，从尾部进行
         * @param taskArr
         * @return
         * @notice 减少锁获取的频率并可能提高性能
         */
        bool trySteal(std::vector<T>& taskArr, int maxStealBatchSize) {
            bool result = false;
            if (!deque_.empty() && lock_.try_lock()) {
                while (!deque_.empty() && maxStealBatchSize--) {
                    taskArr.emplace_back(std::forward<T>(deque_.back()));
                    deque_.pop_back();
                    result = true;
                }
                lock_.unlock();
            }

            return result;    // 如果非空，表示盗取成功
        }
        WorkStealingQueue() = default;
        NO_ALLOWED_COPY(WorkStealingQueue)

    private:
        std::deque<T> deque_;            // 存放任务的双向队列
        std::mutex lock_;                // 用于处理deque_的锁

};

}

#endif WORKSTEALINGQUEUE_H