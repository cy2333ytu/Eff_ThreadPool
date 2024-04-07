#ifndef LOCKFREERINGBUFFERQUEUE_H
#define LOCKFREERINGBUFFERQUEUE_H

#include "QueueObject.h"
#include <atomic>
#include <vector>
#include <memory>

namespace ccy
{

template<typename T, int CAPACITY = DEFAULT_ATOMICRING_SIZE>
class LockFreeRingBufferQueue: public QueueObject{
    public:
        explicit LockFreeRingBufferQueue(){
            head_ = 0;
            tail_ = 0;
            ring_buffer_.resize(CAPACITY);
        }

        ~LockFreeRingBufferQueue() override{
            ring_buffer_.clear();
        }

        /**
         * 写入一个任务
         * @param value
         */
        void push(T&& value){
            int curTail = tail_.load(std::memory_order_relaxed);
            int nextTail = (curTail + 1) % CAPACITY;

            while(nextTail == head_.load(std::memory_order_acquire)){
                // 队列已满，等待其他线程出队
                std::this_thread::yield();
            }
            ring_buffer_[curTail] = std::move(value);
            tail_.store(nextTail, std::memory_order_release);
        }

        /**
         * 尝试弹出一个任务
         * @param value
         * @return
         */
        bool tryPop(T& value){
            int curHead = head_.load(std::memory_order_relaxed);
            if(curHead == tail_.load(std::memory_order_acquire)){
                // 队列已空，直接返回false
                return false;
            }

            value = std::move(ring_buffer_[curHead]);
            int nextHead = (curHead + 1) % CAPACITY;
            head_.store(nextHead, std::memory_order_release);
            return true;
        }
    private:
        std::atomic<int> head_;                                // 开始元素（较早写入的）的位置
        std::atomic<int> tail_;                                // 尾部的位置
        std::vector<std::unique_ptr<T> > ring_buffer_;          // 环形队列

};

}

#endif
