cmake_minimum_required(VERSION 3.2)

project(ThreadPool VERSION 1.2.1)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread -lbenchmark -Wno-format-overflow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread -Wno-format-overflow")

add_definitions(-D_ENABLE_LIKELY_)

file(GLOB SRC_LIST "${CMAKE_SOURCE_DIR}/*.cc")


# 编译libThreadPool动态库
# add_library(ThreadPool SHARED ${SRC_LIST})

# 编译libThreadPool静态库
# add_library(ThreadPool STATIC ${SRC_LIST})

find_package(benchmark REQUIRED)

add_executable(test
        ${SRC_LIST}
        testfunc.cpp)
target_link_libraries(test benchmark::benchmark pthread)
