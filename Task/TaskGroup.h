#ifndef TASKGROUP_H
#define TASKGROUP_H

#include <utility>
#include "../ThreadObject.h"
#include "../Basic/FuncType.h"
namespace ccy
{

class TaskGroup: public ThreadObject{
    public:
        explicit TaskGroup() = default;
        NO_ALLOWED_COPY(TaskGroup)
        /**
         * 直接通过函数来申明taskGroup
         * @param task
         * @param ttl
         * @param onFinished
         */
        explicit TaskGroup(DEFAULT_CONST_FUNCTION_REF task,
                            long ttl = MAX_BLOCK_TTL,
                            CALLBACK_CONST_FUNCTION_REF onFinished = nullptr) noexcept
                {
                    this->addTask(task)
                        ->setTtl(ttl)
                        ->setOnFinished(onFinished);
                }

        /**
         * 添加一个任务
         * @param task
        */
        TaskGroup* addTask(DEFAULT_CONST_FUNCTION_REF task){
            task_arr_.emplace_back(task);
            return this;
        }
        /**
         * 设置任务最大超时时间
         * @param ttl
         */
        TaskGroup* setTtl(long ttl) {
            this->ttl_ = ttl;
            return this;
        }

        /**
         * 设置执行完成后的回调函数
         * @param onFinished
         * @return
         */
        TaskGroup* setOnFinished(CALLBACK_CONST_FUNCTION_REF onFinished) {
            this->on_finished_ = onFinished;
            return this;
        }

        /**
         * 获取最大超时时间信息
         * @return
         */
        long getTtl() const {
            return this->ttl_;
        }

        /**
         * 清空任务组
         */
        void clear() {
            task_arr_.clear();
        }

        /**
         * 获取任务组大小
         * @return
         */
        size_t getSize() const {
            auto size = task_arr_.size();
            return size;
        }
    private:
        std::vector<DEFAULT_FUNCTION> task_arr_;                // 任务消息
        long ttl_ = MAX_BLOCK_TTL;                              // 任务组最大执行耗时(0，表示不阻塞)
        CALLBACK_FUNCTION on_finished_ = nullptr;               // 执行函数任务结束

        friend class ThreadPool;
};
using TaskGroupPtr = TaskGroup *;
using TaskGroupRef = TaskGroup &;

}

#endif