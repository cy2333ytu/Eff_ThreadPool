#include <benchmark/benchmark.h>
#include "ThreadPool.h"
#include "MyFunction.h"

#include <future>
#include <functional>
using namespace ccy;
// 模拟工作负载，例如一个简单的加法
int simpleWork(int a, int b) {
    return a + b; // 替换为所需执行的计算
}

// 基准测试单次提交工作到线程池
static void BM_SimpleThreadPool(benchmark::State& state) {
    ThreadPoolConfig config;
    config.secondary_thread_size_ = 0;

    ThreadPool pool(state.range(0)); // 以state.range(0)作为线程数
    pool.setConfig(config);
    for (auto _ : state) {

        state.PauseTiming();
        std::vector<std::future<int>> futures;
        state.ResumeTiming();

        for (int i = 0; i < state.range(1); ++i) {
            futures.emplace_back(pool.commit(std::bind(simpleWork, i, i)));
        }

        for (auto &f : futures) {
            f.get(); // 等待所有的工作完成
        }
    }
}

// 使用不同的线程数和工作量来运行测试
BENCHMARK(BM_SimpleThreadPool)
    ->Args({1, 1000})       // 1个线程, 1000个工作项
    ->Args({4, 1000})       // 4个线程, 1000个工作项
    ->Args({8, 1000})       // 8个线程, 1000个工作项
    ->Args({1, 10000})      // 1个线程, 10000个工作项
    ->Args({4, 10000})      // 4个线程, 10000个工作项
    ->Args({8, 10000})     // 8个线程, 10000个工作项
    ->Args({8, 100000})     // 8个线程, 100000个工作项
    ->Args({8, 200000})     // 8个线程, 200000个工作项
    ->Args({8, 400000})     // 8个线程, 400000个工作项
    ->Args({8, 800000});     // 8个线程, 800000个工作项

BENCHMARK_MAIN(); // 主函数，启动所有基准测试