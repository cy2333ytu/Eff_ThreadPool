#ifndef FUNCTYPE_H
#define FUNCTYPE_H

#include <functional>
#include "Status.h"
#include "StrDefine.h"

namespace ccy
{

using DEFAULT_CONST_FUNCTION_REF = const std::function<void()>&;
using CALLBACK_CONST_FUNCTION_REF = const std::function<void(Status)>&;
using DEFAULT_FUNCTION = std::function<void()>;
using CALLBACK_FUNCTION = std::function<void(Status)>;



}

#endif