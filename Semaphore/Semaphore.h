#ifndef SEMOPHORE_H
#define SEMOPHORE_H

#include "../ThreadObject.h"

#include <mutex>
#include <condition_variable>

namespace ccy
{

class Semaphore : public ThreadObject{
public:
    /**
     * 触发一次信号
     */
    void signal(){
        UNIQUE_LOCK lk(mutex_);
        cnt_++;
        if(cnt_ < 0){
            cv_.notify_one();
        }
    }
    
    /**
     * 等待信号触发
     */
    void wait(){
        UNIQUE_LOCK lk(mutex_);
        cnt_--;
        if(cnt_ < 0){
            cv_.wait(lk);
        }
    }

    private:
        int cnt_ = 0;
        std::mutex mutex_;
        std::condition_variable cv_;

};

}


#endif