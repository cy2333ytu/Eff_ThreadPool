#ifndef UTILS_DEFINE
#define UTILS_DEFINE

#include "../Basic/Status.h"
#include "../Basic/BasicInclude.h"

#include <mutex>
#include <memory>
#include <iostream>

namespace ccy
{

using LOCK_GUARD = std::lock_guard<std::mutex>;
using UNIQUE_LOCK = std::unique_lock<std::mutex>;
using Exception = ccy::EXCEPTION;

#ifdef _ENABLE_LIKELY_
    #define likely(x)   __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)
#else
    #define likely
    #define unlikely
#endif

template<bool B, typename T = void>
using c_enable_if_t = typename std::enable_if<B, T>::type;

#define NO_ALLOWED_COPY(Type)                                          \
    Type(const Type &) = delete;                                      \
    const Type &operator=(const Type &) = delete;                     \

template<typename T, typename... Args>
typename std::unique_ptr<T> c_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}


/** 获取当前代码所在的位置信息 */
#define GET_LOCATE                                                      \
    (std::string(__FILE__) + " | " + std::string(__FUNCTION__)          \
    + " | line = [" + ::std::to_string( __LINE__) + "]")

#define ErrStatus(info)                                                \
    Status(info, GET_LOCATE)                                            \

/** 返回异常信息和状态 */
#define RETURN_ERROR_STATUS(info)                                       \
    return ErrStatus(info);   

#define ASSERT_INIT(isInit)                                              \
    if (unlikely((isInit) != is_init_)) {                                \
        RETURN_ERROR_STATUS("init status is not suitable")               \
    }                                                                     \

/** 根据条件判断是否返回错误状态 */
#define RETURN_ERROR_STATUS_BY_CONDITION(cond, info)                    \
    if (unlikely(cond)) { RETURN_ERROR_STATUS(info); }                  \


#define DELETE_PTR(ptr)                                                         \
    if (unlikely((ptr) != nullptr)) {                                           \
        delete (ptr);                                                           \
        (ptr) = nullptr;                                                        \
    }                                                                           \

/* 判断函数流程是否可以继续 */
static std::mutex g_check_status_mtx;
#define FUNCTION_CHECK_STATUS                                                               \
    if (unlikely(status.isErr())) {                                                          \
        if (status.isCrash()) { throw Exception(status.getInfo()); }                        \
        LOCK_GUARD lock{ g_check_status_mtx };                                              \
        std::cout << status.getLocate().c_str() << "errorCode = " << status.getCode() << "," \
            << "errorInfo = " << status.getInfo().c_str() << "\n";                           \
        return status;                                                                         \
    }                                                                                           \

template<typename T>
Status __ASSERT_NOT_NULL(T t) {
    return (unlikely(nullptr == t))
           ? ErrStatus(INPUT_IS_NULL)
           : Status();
}

template<typename T, typename... Args>
Status __ASSERT_NOT_NULL(T t, Args... args) {
    if (unlikely(t == nullptr)) {
        return __ASSERT_NOT_NULL(t);
    }

    return __ASSERT_NOT_NULL(args...);
}

/** 判断传入的多个指针信息，是否为空 */
#define ASSERT_NOT_NULL(ptr, ...)                                                           \
    {                                                                                        \
        const Status& __cur_status__ = __ASSERT_NOT_NULL(ptr, ##__VA_ARGS__);               \
        if (unlikely(__cur_status__.isErr())) { return __cur_status__; }                     \
    }                                                                                          \

}
#endif

