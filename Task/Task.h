#ifndef TASK_H
#define TASK_H

#include "../ThreadObject.h"
#include <vector>
#include <memory>

namespace ccy
{

class Task: public ThreadObject{
    struct taskBased
    {
        explicit taskBased() = default;
        virtual void call() = 0;
        virtual ~taskBased() = default;
    };
    
    template<typename F, typename = typename std::decay<F>::type>
    struct taskDerived: taskBased
    {
        T func_;
        explicit taskDerived(F&& func): func_(std::forward<F>(func)){}
        void call() override {func_();}
    };
public:
    template<typename F>
    Task(F&& f, int priority = 0)
        : impl_(new taskDerived<F>(std::forward<F>(f)))
        , priority_(priority){}
    
    void operator()(){
        impl_->call();
    }

    Task() = default;

    Task(Task&& task) noexcept:
        impl_(std::move(task.impl_)),
        priority_(task.priority_) {}

    Task &operator=(Task&& task) noexcept {
        impl_ = std::move(task.impl_);
        priority_ = task.priority_;
        return *this;
    }

    bool operator>(const Task& task) const {
        return priority_ < task.priority_;    // 新加入的，优先级较低，放到后面
    }

    bool operator<(const Task& task) const {
        return priority_ >= task.priority_;
    }

    NO_ALLOWED_COPY(Task)
    private:
        std::unique_ptr<taskBased> impl_ = nullptr;
        int priority_ = 0;
};

using TaskRef = Task &;
using TaskPtr = Task *;
using TaskArr = std::vector<Task>;
using TaskArrRef = std::vector<Task> &;

}

#endif TASK_H