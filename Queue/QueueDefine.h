#ifndef QUEUE_DEFINE_H
#define QUEUE_DEFINE_H


namespace ccy
{

enum class RingBufferPushStrategy{
    WAIT = 1,          // wait for consume.then write
    REPLACE = 2,       // Replace the earliest entered content that has not been consumed
    DROP = 3,          // Drop the current message
};

}
#endif