/***************************
@Desc: 所有类型的父节点，其中run()方法必须实现
***************************/

#ifndef CGRAPH_COBJECT_H
#define CGRAPH_COBJECT_H

#include "Status.h"
#include "../Utils/UtilsDefine.h"

namespace ccy
{

class Object {
public:
    /**
     * 默认构造函数
     */
    explicit Object() = default;

    /**
     * 初始化函数
     */
    virtual Status init() {
        EMPTY_FUNCTION
    }

    /**
     * 流程处理函数
     */
    virtual STATUS run() = 0;

    /**
     * 释放函数
     */
    virtual Status destroy() {
        EMPTY_FUNCTION
    }

    /**
     * 默认析构函数
     */
    virtual ~Object() = default;
};


}
#endif
