cmake_minimum_required(VERSION 3.2)

project(Benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread -Wno-format-overflow")

add_definitions(-D_ENABLE_LIKELY_)

# file(GLOB SRC_LIST "${CMAKE_SOURCE_DIR}/*.cc")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
find_package(benchmark REQUIRED)

# add_executable(bench1
#         ${SRC_LIST}
#         bench1.cc
#         ../ThreadPool.cc
#         )
# target_link_libraries(bench1 benchmark::benchmark pthread)

add_executable(bench2
        ${SRC_LIST}
        bench2.cc
        ../ThreadPool.cc
        )
target_link_libraries(bench2 benchmark::benchmark pthread)

add_executable(bench3
        ${SRC_LIST}
        bench3.cc
        ../ThreadPool.cc
        )
target_link_libraries(bench3 benchmark::benchmark pthread)
