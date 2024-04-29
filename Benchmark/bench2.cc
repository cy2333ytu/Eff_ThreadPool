#include "../ThreadPool.h"
#include "timewait.h"
#include <iomanip>
#include <benchmark/benchmark.h>
#include <future>
#include <functional>
using namespace ccy;
/*
打包执行
*/

int main(int argn, char** argvs) {
    int task_nums, thread_nums;
    thread_nums = atoi(argvs[2]);
    task_nums = atoi(argvs[1]);

    ThreadPoolConfig config;
    config.batch_task_enable_ = true;
    config.secondary_thread_size_ = 0;
    ThreadPool pool; 
    // std::vector<std::future<void>> futures; // 存放返回的future
    TaskGroup taskGroup;
    Status a;
    auto time_cost = timewait([&]{
        auto task = []{/* empty task */};
            for (int i = 0; i < task_nums/10; ++i) {
                for (int i = 0; i < 10; ++i) {
                    taskGroup.addTask(task);
                }
                a = pool.submit(taskGroup);
            }
            // std::cout << a.getCode() << "\n";
    });
    std::cout<<"threads: "<<std::left<<std::setw(2)<<thread_nums<<" |  tasks: "<<task_nums<<"  |  time-cost: "<<time_cost<<" (s)"<<std::endl;

}

