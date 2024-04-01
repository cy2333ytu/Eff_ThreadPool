#ifndef THREADPOOLDEFINE_H
#define THREADPOOLDEFINE_H

namespace ccy
{
static const unsigned int DEFAULT_RINGBUFFER_SIZE = 1024;                     // 默认环形队列的大小
static const unsigned int DEFAULT_ATOMICRING_SIZE = 1024;                    // 默认环形队列的大小
static const int SECONDARY_THREAD_COMMON_ID = -1;                            // 辅助线程统一id标识
static const int THREAD_TYPE_PRIMARY = 1;
static const long MAX_BLOCK_TTL = 1999999999;                        // 最大阻塞时间，单位为ms
}
#endif