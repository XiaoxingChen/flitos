#if !defined(_CS251_CONDITION_VARIABLE_H_)
#define _CS251_CONDITION_VARIABLE_H_

namespace cs251
{
using cond_id_t = int;

class ConditionVariableInternal
{

public:
    ConditionVariableInternal() = default;
    ConditionVariableInternal(cond_id_t cond_id): cond_id_(cond_id){}
    void wait(mutex_id_t mtx_id)
    {
        // assert lock is acquired
        schedulerInstance().setPreemption(false);
        LOGD("wait on cond: thread %d, mtx: %d\n", schedulerInstance().runningThreadID(), mtx_id);
        mutexFactoryInstance().unlock(mtx_id);
        waiting_list_.push_back(schedulerInstance().runningThreadID());
        schedulerInstance().suspend();
        schedulerInstance().setPreemption(true);
        mutexFactoryInstance().lock(mtx_id);
    }

    void notifyOne()
    {
        schedulerInstance().setPreemption(false);
        do
        {
            if(waiting_list_.empty()) break;
            // LOGD("%s:%d\n", __FILE__, __LINE__);
            thread_id_t tid = waiting_list_.front();
            LOGD("thread %d put to ready list by condition variable %d\n", tid, cond_id_);
            waiting_list_.pop_front();
            schedulerInstance().resume(tid);
        } while (0);
        schedulerInstance().setPreemption(true);
    }

    void notifyAll()
    {
        while(!waiting_list_.empty()) notifyOne();
    }
private:
    cond_id_t cond_id_ = -1;
    ecs::deque<thread_id_t, ecs::allocator_nosys<thread_id_t>> waiting_list_;
};

class ConditionVariableFactory
{
public:
    cond_id_t create() 
    {  
        seq_id_++;
        id_map_[seq_id_] = ConditionVariableInternal(seq_id_);
        return seq_id_;
    }
    void destroy(cond_id_t cond_id)
    {
        id_map_.erase(cond_id);
    }
    void wait(cond_id_t id, mutex_id_t mtx_id) 
    {
        // LOGD("wait: thread %d, mtx: %d, cond: %d \n", schedulerInstance().runningThreadID(), mtx_id, id);
        id_map_[id].wait(mtx_id); 
    }
    void notifyOne(cond_id_t id) { id_map_[id].notifyOne(); }
    void notifyAll(cond_id_t id) { id_map_[id].notifyAll(); }
private:
    cond_id_t seq_id_ = 0;
    ecs::map<cond_id_t, ConditionVariableInternal> id_map_; 
};

extern void* g_condition_variable_factory;

inline ConditionVariableFactory& condFactoryInstance()
{
    if(g_condition_variable_factory == nullptr)
        g_condition_variable_factory = (void*) new ConditionVariableFactory();
    return *(ConditionVariableFactory*)g_condition_variable_factory;
}


} // namespace cs251



#endif // _CS251_CONDITION_VARIABLE_H_
