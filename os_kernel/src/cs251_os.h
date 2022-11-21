#if !defined(_CS251_OS_H)
#define _CS251_OS_H

#include <stdlib.h>
#include <stdint.h>
#include "ecs_list.h"
#include "ecs_map.h"
#include "port_riscv.h"

#if defined(VIRT_CLINT)
#include "uart_printf.h"
#else
#define LOGD(...) while(0)
#endif // VIRT_CLINT


extern "C" void context_switch(volatile size_t** oldsp, volatile size_t* newsp);
extern "C" void disable_interrupts();
extern "C" void enable_interrupts();
extern "C" void startFirstTask( uint32_t stk_ptr );
extern "C" uint8_t __global_pointer$[];


namespace cs251
{
enum ThreadState
{
    eINIT = 0,
    eREADY,
    eRUNNING,
    eWAITING,
    eFINISHED
};

// constexpr size_t SIZE_OF_POPAD = 14;
constexpr size_t INITIAL_STACK_SIZE = 0x1000;
using thread_id_t = int;
using mutex_id_t = int;

class ThreadControlBlock;
class ThreadScheduler;
class MutexFactory;

void stub_wrapper(void (*f)(void*), void* arg);
void thread_switch(ThreadControlBlock& curr_tcb, ThreadControlBlock& next_tcb);
inline ThreadScheduler& schedulerInstance();
inline MutexFactory& mutexFactoryInstance();


class ThreadControlBlock
{
public:
    ThreadControlBlock(){ }
    // thread_id_t id() const { return thread_id_; }
    const ThreadState& state() const { return state_; }
    void* pc() { return program_counter_; }
    size_t*& sp() { return stack_ptr_; }
    void setState(const ThreadState& state) { state_ = state; }

    void pushDummySwitchFrame()
    {
        if(!stack_ptr_) return;
        stack_ptr_ -= static_cast<size_t>(context_size);
        *(stack_ptr_ + static_cast<size_t>(offset_ra)) = reinterpret_cast<size_t>(stub_wrapper);
        *(stack_ptr_ + static_cast<size_t>(offset_gp)) = reinterpret_cast<size_t>(__global_pointer$);
    }
#if 0
    void init(void (*f)(void*), void* arg)
    {
        // thread_id_ = threadCounter();
        stk_size_ = INITIAL_STACK_SIZE;
        stk_mem_ = (uint8_t*)malloc(INITIAL_STACK_SIZE);
        stack_ptr_ = reinterpret_cast<size_t*>(stk_mem_ + INITIAL_STACK_SIZE) - 1; //todo
        program_counter_ = reinterpret_cast<void*>(stub_wrapper);

        pushDummySwitchFrame();
        *(stack_ptr_ + static_cast<size_t>(offset_a1)) = reinterpret_cast<size_t>(arg);
        *(stack_ptr_ + static_cast<size_t>(offset_a0)) = reinterpret_cast<size_t>(f);
        state_ = ThreadState::eREADY;
    }
#else
    void init(void (*f)(void*), void* arg);
#endif

    ~ThreadControlBlock()
    {
        if(stk_mem_)
        {
            free(stk_mem_);
            stk_mem_ = nullptr;
        }
    }
    // static size_t& threadCounter()
    // {
    //     static size_t thread_counter = 0;
    //     return thread_counter;
    // }

    mutex_id_t mutexForJoin() const { return mtx_for_join_; }

private:
    
    // thread_id_t thread_id_ = 0;
    size_t stk_size_ = 0;
    uint8_t* stk_mem_ = nullptr;
    size_t* stack_ptr_ = nullptr;
    void* program_counter_ = nullptr;
    ThreadState state_ = eINIT;
    mutex_id_t mtx_for_join_ = -1;
};



class ThreadScheduler
{
public:
    void clearFinishedList()
    {
        for(auto it = finished_list_.begin(); it != finished_list_.end(); it++)
        {
            id_tcb_map_.erase(*it);
        }
        finished_list_.clear();
    }

    thread_id_t create(void (*f)(void*), void* arg)
    {
        thread_counter_++;
        thread_id_t id = thread_counter_;
        id_tcb_map_[id] = ThreadControlBlock();
        id_tcb_map_[id].init(f, arg);
        ready_list_.push_back(id);
        LOGD("create thread: %d\n", id);
        return id;
    }

    void launchFirstTask()
    {
        if(ready_list_.empty()) return; // idle thread should always be in ready_list?
        thread_id_t chosen_id = ready_list_.front();
        ready_list_.pop_front();

        id_tcb_map_[chosen_id].setState(ThreadState::eRUNNING);
        running_thread_id_ = chosen_id;
        setPreemption(true);
        startFirstTask((uint32_t) (id_tcb_map_[chosen_id].sp()));
    }

    void switchCurrentThreadTo(const ThreadState& state)
    {
        // Voluntary context switch and 
        // involuntary context switch 
        // could generate race condition.
        // Therefore disable interrupts here.
        setPreemption(false);
        
        if(state != ThreadState::eREADY 
        && state != ThreadState::eWAITING
        && state != ThreadState::eFINISHED) return;
        
        if(ready_list_.empty()) return; // running thread is idel spin thread

        thread_id_t prev_thread_id = running_thread_id_;
        running_thread_id_ = ready_list_.front();
        ready_list_.pop_front();

        id_tcb_map_[prev_thread_id].setState(state);
        id_tcb_map_[running_thread_id_].setState(ThreadState::eRUNNING);

        if(state == ThreadState::eREADY)
        {
            ready_list_.push_back(prev_thread_id);
        }else if (state == ThreadState::eFINISHED)
        {
            finished_list_.push_back(prev_thread_id);
        }
        
        LOGD("voluntary context switch from %d to %d\n", prev_thread_id, running_thread_id_);
        disable_interrupts();
        thread_switch(id_tcb_map_[prev_thread_id], id_tcb_map_[running_thread_id_]);
        enable_interrupts();
        setPreemption(true);
    }

    void inInterruptYield()
    {
        if(!preemption_on_) return;
        if(ready_list_.empty()) return; // running thread is idel spin thread

        thread_id_t prev_thread_id = running_thread_id_;
        running_thread_id_ = ready_list_.front();
        ready_list_.pop_front();

        id_tcb_map_[prev_thread_id].setState(ThreadState::eREADY);
        id_tcb_map_[running_thread_id_].setState(ThreadState::eRUNNING);

        ready_list_.push_back(prev_thread_id);
        LOGD("in interrupt context switch from %d to %d\n", prev_thread_id, running_thread_id_);
        thread_switch(id_tcb_map_[prev_thread_id], id_tcb_map_[running_thread_id_]);

        // come back from other thread
        schedulerInstance().setPreemption(true);
    }

    void yield()
    {
        switchCurrentThreadTo(ThreadState::eREADY);
    }

    void suspend()
    {
        switchCurrentThreadTo(ThreadState::eWAITING);
    }

    void resume( thread_id_t tid )
    {
        if(id_tcb_map_.count(tid) > 0)
        {
            id_tcb_map_[tid].setState(ThreadState::eREADY);
            ready_list_.push_back(tid);
        }
    }

    void join( thread_id_t tid );

    void setPreemption(bool val) 
    {
        LOGD("thread %d set preemtion to %d\n",runningThreadID(), val); 
        preemption_on_ = val; 
    }


    void exit()
    {
        switchCurrentThreadTo(ThreadState::eFINISHED);
    }

    thread_id_t runningThreadID() const { return running_thread_id_; }

    const ecs::list<thread_id_t>& readyList() const { return ready_list_; }
    const ecs::map<thread_id_t, ThreadControlBlock>& idTcbMap() const { return id_tcb_map_; }
private:
    bool preemption_on_ = false;
    thread_id_t running_thread_id_ = 0;
    size_t thread_counter_ = 0;
    ecs::list<thread_id_t> ready_list_;
    // ecs::list<thread_id_t> waiting_list_;
    ecs::list<thread_id_t> finished_list_;
    ecs::map<thread_id_t, ThreadControlBlock> id_tcb_map_;
};

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
                LOGD("lock hold by thread: %d, suspend current thread: %d\n", owner_, schedulerInstance().runningThreadID());
                waiting_list_.push_back(schedulerInstance().runningThreadID());
                schedulerInstance().suspend();
            }
            
        }else
        {
            owner_ = schedulerInstance().runningThreadID();
            LOGD("lock acquired by %d\n", owner_);
        }
        schedulerInstance().setPreemption(true);
    }

    void unlock()
    {
        schedulerInstance().setPreemption(false);
        LOGD("unlock by thread %d\n", schedulerInstance().runningThreadID());
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
            LOGD("wake thread %d as owner\n", owner_);
            waiting_list_.pop_front();
            schedulerInstance().resume(owner_);
        }
        schedulerInstance().setPreemption(true);
    }
private:
    bool isLocked() { return owner_ >= 0; }

private:
    thread_id_t owner_ = -1;
    ecs::list<thread_id_t> waiting_list_;
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

inline void thread_switch(ThreadControlBlock& curr_tcb, ThreadControlBlock& next_tcb)
{
    context_switch(const_cast<volatile size_t**>(&curr_tcb.sp()), next_tcb.sp());
}

extern void* g_scheduler_;

inline ThreadScheduler& schedulerInstance()
{
    if(g_scheduler_ == nullptr)
    {
        g_scheduler_ = reinterpret_cast<void*>(new ThreadScheduler());
    }
    return *((cs251::ThreadScheduler*)g_scheduler_);
}


inline int thread_exit()
{
    schedulerInstance().exit();
    return 0;
} 

inline void stub_wrapper(void (*f)(void*), void* arg)
{
    // increaseTimeCompare(1000);
    enable_interrupts();
    (*f)(arg);
    
    mutex_id_t mtx_for_join = schedulerInstance().idTcbMap().at(schedulerInstance().runningThreadID()).mutexForJoin();
    mutexFactoryInstance().unlock( mtx_for_join );
    thread_exit();
}

// 4.6.1 Creating a Thread
inline int thread_create(void (*f)(void), void* arg)
{
    return 0;
} 

inline int thread_yield()
{
    schedulerInstance().yield();
    return 0;
}

inline void ThreadControlBlock::init(void (*f)(void*), void* arg)
{
    // thread_id_ = threadCounter();
    stk_size_ = INITIAL_STACK_SIZE;
    stk_mem_ = (uint8_t*)malloc(INITIAL_STACK_SIZE);
    stack_ptr_ = reinterpret_cast<size_t*>(stk_mem_ + INITIAL_STACK_SIZE) - 1; //todo
    program_counter_ = reinterpret_cast<void*>(stub_wrapper);

    pushDummySwitchFrame();
    *(stack_ptr_ + static_cast<size_t>(offset_a1)) = reinterpret_cast<size_t>(arg);
    *(stack_ptr_ + static_cast<size_t>(offset_a0)) = reinterpret_cast<size_t>(f);
    state_ = ThreadState::eREADY;

    mtx_for_join_ = mutexFactoryInstance().create();
    mutexFactoryInstance().lock(mtx_for_join_);
}

inline void ThreadScheduler::join( thread_id_t tid )
{
    mutex_id_t mtx_for_join = id_tcb_map_[tid].mutexForJoin();
    mutexFactoryInstance().lock(mtx_for_join);
    mutexFactoryInstance().unlock(mtx_for_join);
}

#ifdef CS251_OS_STATIC_OBJECTS_ON
void* g_scheduler_ = nullptr;
void* g_mutex_factory = nullptr;
#endif

} // namespace cs251

#endif // _CS251_OS_H
