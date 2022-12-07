#if !defined(_CS251_MUTEX_H_)
#define _CS251_MUTEX_H_

#include "cs251_scheduler.h"

namespace cs251
{
using mutex_id_t = int;

class MutexInternal
{
public:
    MutexInternal() {  }
    bool try_lock()
    {
        bool ret = true;
        schedulerInstance().setPreemption(false);
        if(isLocked())
        {
            ret = false;
        }else
        {
            owner_ = schedulerInstance().runningThreadID();
            ret = true;
        }
        schedulerInstance().setPreemption(true);
        return ret;
    }

    void lock()
    {
        schedulerInstance().setPreemption(false);
        if(isLocked())
        {
            if(schedulerInstance().runningThreadID() == owner_)
            {
                // lock twice
                // deal lock would occur
            }else
            {
                // LOGD("lock hold by thread: %d, suspend current thread: %d\n", owner_, schedulerInstance().runningThreadID());
                waiting_list_.push_back(schedulerInstance().runningThreadID());
                schedulerInstance().suspend();
            }
            
        }else
        {
            owner_ = schedulerInstance().runningThreadID();
            // LOGD("lock acquired by %d\n", owner_);
        }
        schedulerInstance().setPreemption(true);
    }

    void unlock()
    {
        schedulerInstance().setPreemption(false);
        // LOGD("unlock by thread %d\n", schedulerInstance().runningThreadID());
        if(waiting_list_.empty())
        {
            if(schedulerInstance().runningThreadID() == owner_)
            {
                owner_ = -1;
            }else
            {
                // undefined behavior
            }
            
        }else
        {
            owner_ = waiting_list_.front();
            // LOGD("wake thread %d as owner\n", owner_);
            waiting_list_.pop_front();
            schedulerInstance().resume(owner_);
        }
        schedulerInstance().setPreemption(true);
    }
private:
    bool isLocked() { return owner_ >= 0; }

private:
    thread_id_t owner_ = -1;
    ecs::deque<thread_id_t, ecs::allocator_nosys<thread_id_t>> waiting_list_;
};

class MutexFactory
{
public:
    mutex_id_t create() 
    {  
        seq_id_++;
        mtx_map_[seq_id_] = MutexInternal();
        return seq_id_;
    }
    void destroy(mutex_id_t mtx_id)
    {
        mtx_map_.erase(mtx_id);
    }
    void lock(mutex_id_t id) 
    {
        mtx_map_[id].lock(); 
    }
    void try_lock(mutex_id_t id) { mtx_map_[id].try_lock(); }
    void unlock(mutex_id_t id) 
    { 
        mtx_map_[id].unlock(); 
    }
private:
    mutex_id_t seq_id_ = 0;
    ecs::map<mutex_id_t, MutexInternal> mtx_map_; 
};

extern void* g_mutex_factory;

inline MutexFactory& mutexFactoryInstance()
{
    if(g_mutex_factory == nullptr)
        g_mutex_factory = (void*) new MutexFactory();
    return *(MutexFactory*)g_mutex_factory;
}

} // namespace cs251



#endif // _CS251_MUTEX_H_
