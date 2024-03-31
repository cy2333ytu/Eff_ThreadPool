#ifndef QUEUE_OBJECT_H
#define QUEUE_OBJECT_H

#include "../ThreadObject.h"
#include "QueueDefine.h"

#include <mutex>
#include <condition_variable>


namespace ccy
{

class QueueOject: public ThreadObject{
    protected:
        std::mutex mutex_;
        std::condition_variable cv_;
};

}

#endif