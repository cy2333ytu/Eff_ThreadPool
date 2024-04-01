#ifndef UTILS_DEFINE
#define UTILS_DEFINE

#include "../Basic/Status.h"
#include <mutex>
#include <memory>


using Status = ccy::STATUS;
using LOCK_GUARD = std::lock_guard<std::mutex>;
using UNIQUE_LOCK = std::unique_lock<std::mutex>;

#define NO_ALLOWED_COPY(Type)                                          \
    Type(const Type &) = delete;                                      \
    const Type &operator=(const Type &) = delete;                     \

template<typename T, typename... Args>
typename std::unique_ptr<T> c_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/** 开启函数流程 */
#define FUNCTION_BEGIN                                                  \
    CStatus status;                                                     \

/** 结束函数流程 */
#define FUNCTION_END                                                    \
    return status;                                                      \

/** 获取当前代码所在的位置信息 */
#define GET_LOCATE                                                      \
    (std::string(__FILE__) + " | " + std::string(__FUNCTION__)          \
    + " | line = [" + ::std::to_string( __LINE__) + "]")

#define ErrStatus(info)                                                \
    Status(info, CGRAPH_GET_LOCATE)                                    \

/** 返回异常信息和状态 */
#define RETURN_ERROR_STATUS(info)                                       \
    return ErrStatus(info);   

#define CGRAPH_ASSERT_INIT(isInit)                                              \
    if (unlikely((isInit) != is_init_)) {                                       \
        CGRAPH_RETURN_ERROR_STATUS("init status is not suitable")               \
    }                                                                           \

#endif