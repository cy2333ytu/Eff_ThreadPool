#include "QueueObject.h"
#include <iostream>
#include <vector>
#include <atomic>
#include <chrono>

namespace ccy{

template<typename T, unsigned int capacity = DEFAULT_RINGBUFFER_SIZE>
class AtomicRingBufferQueue: public QueueObject{
    public:
        explicit AtomicRingBufferQueue(){
            head_ = 0;
            tail_ = 0;
            capacity_ = capacity;
            ring_buffer_queue_.resize(capacity_); 
        }
        ~AtomicRingBufferQueue() override{
            clear();
        }

        /**
         * 设置容量信息
         * @param size
         * @return
         * @notice push信息之后，不要使用
         */
        AtomicRingBufferQueue* setCapacity(unsigned int size) {
            capacity_ = size;
            ring_buffer_queue_.resize(capacity_);
            return this;
        }

        /**
         * 获取容量信息
         * @return
         */
        unsigned int getCapacity() const {
            return capacity_;
        }

        /**
         * 写入信息
         * @tparam TImpl
         * @param value
         * @param strategy
         * @return
         */
        template<class TImpl = T>
        void push(const TImpl& value, RingBufferPushStrategy strategy){
            {
                UNIQUE_LOCK lk(mutex_);
                if(isFull()){
                    switch(strategy){
                        case RingBufferPushStrategy::WAIT:
                            push_cv_.wait(lk, [this] {return !isFull();});
                            break;
                        case RingBufferPushStrategy::REPLACE:
                            head_ = (head_ + 1) % capacity_;
                            break;
                        case RingBufferPushStrategy::DROP:
                            return;                       // 直接返回，不写入
                    }
                }
                ring_buffer_queue_[tail_] = std::move(c_make_unique<TImpl>(value));
                tail_ = (tail_ + 1) % capacity_;
            }
            pop_cv_.notify_one();
        }

        /**
         * 写入智能指针类型的信息
         * @tparam TImpl
         * @param value
         * @param strategy
         * @return
         */
        template<class TImpl = T>
        CVoid push(std::unique_ptr<TImpl>& value, URingBufferPushStrategy strategy) {
            {
                CGRAPH_UNIQUE_LOCK lk(mutex_);
                if (isFull()) {
                    switch (strategy) {
                        case URingBufferPushStrategy::WAIT:
                            push_cv_.wait(lk, [this] { return !isFull(); });
                            break;
                        case URingBufferPushStrategy::REPLACE:
                            head_ = (head_ + 1) % capacity_;
                            break;
                        case URingBufferPushStrategy::DROP:
                            return;    // 直接返回，不写入即可
                    }
                }

                ring_buffer_queue_[tail_] = std::move(value);
                tail_ = (tail_ + 1) % capacity_;
            }
            pop_cv_.notify_one();
        }
        
        /**
         * 等待弹出信息
         * @param value
         * @param timeout
         * @return
         */
        template<class TImpl = T>
        Status waitPopWithTimeout(TImpl& value, long timeout){
            Status status;
            {
                UNIQUE_LOCK lk(mutex_);
                if(isEmpty()
                    && !pop_cv_.wait_for(lk, std::chrono::milliseconds(timeout),
                                     [this] { return !isEmpty();}))
                {
                    RETURN_ERROR_STATUS("receive message timeout")
                }
                value = *ring_buffer_queue_[head_];
                head_ = (head_ + 1) % capacity_;
            }
            push_cv_.notify_one();
            return status;
        }
        /**
         * 等待弹出信息: 传入的参数为智能指针
         * @param value
         * @param timeout
         * @return
         */
        template<class TImpl = T>
        Status waitPopWithTimeout(TImpl& value, long timeout){
            Status status;
            {
                UNIQUE_LOCK lk(mutex_);
                if(isEmpty()
                    && !pop_cv_.wait_for(lk, std::chrono::milliseconds(timeout),
                                     [this] { return !isEmpty();}))
                {
                    RETURN_ERROR_STATUS("receive message timeout")
                }
                value = std::move(ring_buffer_queue_[head_]);
                head_ = (head_ + 1) % capacity_;
            }
            push_cv_.notify_one();
            return status;
        }
        /**
         * 清空所有的数据
         * @return
         */
        Status clear() {
            Status status;
            ring_buffer_queue_.resize(0);
            head_ = 0;
            tail_ = 0;
            return status;
        }
    protected:
        /**
         * 当前队列是否为满
         * @return
         */
        bool isFull() {
            // 空出来一个位置，这个时候不让 tail写入
            return head_ == (tail_ + 1) % capacity_;
        }

        /**
         * 当前队列是否为空
         * @return
         */
        bool isEmpty() {
            return head_ == tail_;
        }
     
        NO_ALLOWED_COPY(AtomicRingBufferQueue)

    private:
        unsigned int head_;                                             // 头结点位置
        unsigned int tail_;                                             // 尾结点位置
        unsigned int capacity_;                                         // 环形缓冲的容量大小

        std::condition_variable push_cv_;                               // 写入的条件变量.
        std::condition_variable pop_cv_;                                // 读取的条件变量

        std::vector<std::unique_ptr<T> > ring_buffer_queue_;            // 环形缓冲区
};

}