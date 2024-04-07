#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "./Basic/BasicInclude.h"
#include "./Utils/UtilsDefine.h"

#include <mutex>
#include <memory>

namespace ccy
{
/**
 * 仅用于生成Object类型的类
 */
class Allocator : public Object {
public:
    /**
     * 生成一个 Object 对象
     * @tparam T
     * @return
     */
    template<typename T,
            c_enable_if_t<std::is_base_of<Object, T>::value, int> = 0>
    static T* safeMallocCObject() {
        return safeMalloc<T>();
    }

    /**
     * 生成一个 CStruct 的对象
     * @tparam T
     * @return
     */
    template<typename T,
            c_enable_if_t<std::is_base_of<Struct, T>::value, int> = 0>
    static T* safeMallocCStruct() {
        return safeMalloc<T>();
    }

    /**
     * 生成带参数的普通指针
     * @tparam T
     * @tparam Args
     * @param args
     * @return
     */
    template<typename T, typename ...Args,
            c_enable_if_t<std::is_base_of<Object, T>::value, int> = 0>
    static T* safeMallocTemplateCObject(Args... args) {
        T* ptr = nullptr;
        while (!ptr) {
            ptr = new T(std::forward<Args>(args)...);
        }
        return ptr;
    }


    /**
     * 生成unique智能指针信息
     * @tparam T
     * @return
     */
    template<typename T,
            c_enable_if_t<std::is_base_of<Object, T>::value, int> = 0>
    static std::unique_ptr<T> makeUniqueCObject() {
        return c_make_unique<T>();
    }

private:
    /**
     * 生成T类型的对象
     * @tparam T
     * @return
     */
    template<class T>
    static T* safeMalloc() {
        T* ptr = nullptr;
        while (!ptr) {
            ptr = new(std::nothrow) T();
        }
        return ptr;
    }
};


#define SAFE_MALLOC_OBJECT(Type)                                \
    Allocator::safeMallocCObject<Type>();                       \

#define MAKE_UNIQUE_OBJECT(Type)                                \
    Allocator::makeUniqueCObject<Type>();                       \

}
#endif //CGRAPH_UALLOCATOR_H
