#ifndef THREAD_OBJECT_H
#define THREAD_OBJECT_H

#include "Utils/UtilsObject.h"
#include "ThreadPoolDefine.h"

namespace ccy
{
    class ThreadObject: public UtilsObject{
        protected:
            Status run() override{

            }
    };


}

#endif