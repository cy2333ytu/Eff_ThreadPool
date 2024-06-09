#include <benchmark/benchmark.h>
#include "../ThreadPool.h"
#include <future>
#include <random>
#include <chrono>
#include <iostream>
#include <vector>
#include <memory>
using namespace ccy;

static void simulate_workload(benchmark::State& state, int num_threads, int num_tasks, double io_task_ratio) {
    std::unique_ptr<ThreadPool> pool(new ThreadPool(num_threads)); 

    ThreadPoolConfig config;
    config.secondary_thread_size_ = 16;
    int prio = -10;
    std::default_random_engine generator;
    std::bernoulli_distribution distribution(io_task_ratio);

    auto compute_task = [] {
        std::vector<int> primes;
        for (int num = 2; num < 1000; ++num) {
            bool is_prime = true;
            for (int factor = 2; factor * factor <= num; ++factor) {
                if (num % factor == 0) {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime) {
                primes.push_back(num);
            }
        }
    };

    auto io_task = [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    for (auto _ : state) {
        std::vector<std::future<void>> futures;
        for (int i = 0; i < num_tasks; ++i) {
            if (distribution(generator)) {
                futures.emplace_back(pool->commitWithPriority(io_task, prio));
            } else {
                futures.emplace_back(pool->commit(compute_task));
            }
        }

        for (auto &f : futures) {
            f.get();
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <num_threads> <num_tasks> <io_task_ratio>" << std::endl;
        return 1;
    }

    int num_threads = std::stoi(argv[1]);
    int num_tasks = std::stoi(argv[2]);
    double io_task_ratio = std::stod(argv[3]);

    int remaining_argc = 1; 
    for (int i = 4; i < argc; ++i) {
        argv[remaining_argc++] = argv[i];
    }

    argc = remaining_argc;

    benchmark::RegisterBenchmark("BM_MixedWorkload", [num_threads, num_tasks, io_task_ratio](benchmark::State& state) {
        simulate_workload(state, num_threads, num_tasks, io_task_ratio);
    });

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1; // This should not fail now
    benchmark::RunSpecifiedBenchmarks();
}