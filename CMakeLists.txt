cmake_minimum_required(VERSION 3.2)

project(ThreadPool VERSION 1.2.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread -Wno-format-overflow")

add_definitions(-D_ENABLE_LIKELY_)

file(GLOB_RECURSE CTP_SRC_LIST "*.cc")


# 编译libThreadPool动态库
# add_library(ThreadPool SHARED ${CTP_SRC_LIST})

# 编译libThreadPool静态库
# add_library(ThreadPool STATIC ${CTP_SRC_LIST})


add_executable(EThreadPool
        ${CTP_SRC_LIST}
        test.cpp)
