cmake_minimum_required(VERSION 3.2)

project(Benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread -Wno-format-overflow")

add_definitions(-D_ENABLE_LIKELY_)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
find_package(benchmark REQUIRED)

add_executable(benchmark
        ${SRC_LIST}
        benchmark.cpp
        ../ThreadPool.cc
        )
target_link_libraries(benchmark benchmark::benchmark pthread)

add_executable(benchmarkOther
        ${SRC_LIST}
        benchmarkOther.cpp
        ./other/ThreadPool/ThreadPool.h
        )
target_link_libraries(benchmarkOther benchmark::benchmark pthread)