#ifndef UTILS_DEFINE
#define UTILS_DEFINE

#include "../Basic/Status.h"
#include <mutex>
#include <memory>


using Status = ccy::STATUS;
using LOCK_GUARD = std::lock_guard<std::mutex>;
using UNIQUE_LOCK = std::unique_lock<std::mutex>;

#define NO_ALLOWED_COPY(Type)                                   \
    Type(const Type &) = delete;                                      \
    const Type &operator=(const Type &) = delete;                     \

template<typename T, typename... Args>
typename std::unique_ptr<T> c_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif