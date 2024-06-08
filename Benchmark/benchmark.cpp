#include <benchmark/benchmark.h>
#include "../ThreadPool.h"
#include <future>
#include <functional>
using namespace ccy;


// 基准测试单次提交工作到线程池
static void BM_SimpleThreadPool(benchmark::State& state) {
    ThreadPoolConfig config;
    config.secondary_thread_size_ = 4;

    ThreadPool pool(state.range(0)); // 以state.range(0)作为线程数
    pool.setConfig(config);
    for (auto _ : state) {

        state.PauseTiming();
        std::vector<std::future<void>> futures;
        state.ResumeTiming();

        for (int i = 0; i < state.range(1); ++i) {
            auto task = []{};
            futures.emplace_back(pool.commit(task));
        }

        for (auto &f : futures) {
            f.get(); // 等待所有的工作完成
        }
    }
}

// 使用不同的线程数和工作量来运行测试
BENCHMARK(BM_SimpleThreadPool)
    ->Args({16, 10000})     // 16个线程, 10000个工作项
    ->Args({16, 100000})     // 16个线程, 100000个工作项
    ->Args({16, 200000})     // 16个线程, 200000个工作项
    ->Args({16, 400000})     // 16个线程, 400000个工作项
    ->Args({16, 800000})     // 16个线程, 800000个工作项
    ->Args({16, 1000000});     // 16个线程, 1000000个工作项

BENCHMARK_MAIN(); // 主函数，启动所有基准测试