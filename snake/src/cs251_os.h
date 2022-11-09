#if !defined(_CS251_OS_H)
#define _CS251_OS_H

#include <stdlib.h>
#include <stdint.h>
#include "ecs_list.h"
#include "ecs_map.h"

extern "C" void context_switch(volatile size_t** oldsp, volatile size_t* newsp);
extern "C" void disable_interrupts();
extern "C" void enable_interrupts();
extern "C" void startFirstTask( uint32_t stk_ptr );


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

constexpr size_t SIZE_OF_POPAD = 13;
constexpr size_t INITIAL_STACK_SIZE = 0x1000;
using thread_id_t = int;
using mutex_id_t = int;

class ThreadControlBlock;
class ThreadScheduler;
void stub_wrapper(void (*f)(void*), void* arg);
void thread_switch(ThreadControlBlock& curr_tcb, ThreadControlBlock& next_tcb);
inline ThreadScheduler& schedulerInstance();


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
        stack_ptr_ -= SIZE_OF_POPAD;
        *(stack_ptr_ + 12) = reinterpret_cast<size_t>(stub_wrapper);
    }

    void init(void (*f)(void*), void* arg)
    {
        // thread_id_ = threadCounter();
        stk_size_ = INITIAL_STACK_SIZE;
        stk_mem_ = (uint8_t*)malloc(INITIAL_STACK_SIZE);
        stack_ptr_ = reinterpret_cast<size_t*>(stk_mem_ + INITIAL_STACK_SIZE) - 1; //todo
        program_counter_ = reinterpret_cast<void*>(stub_wrapper);

        pushDummySwitchFrame();
        *(stack_ptr_ + 4) = reinterpret_cast<size_t>(arg);
        *(stack_ptr_ + 5) = reinterpret_cast<size_t>(f);
        state_ = ThreadState::eREADY;
    }

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

private:
    
    // thread_id_t thread_id_ = 0;
    size_t stk_size_ = 0;
    uint8_t* stk_mem_ = nullptr;
    size_t* stack_ptr_ = nullptr;
    void* program_counter_ = nullptr;
    ThreadState state_ = eINIT;
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
        return id;
    }

    void launchFirstTask()
    {
        if(ready_list_.empty()) return; // idle thread should always be in ready_list?
        thread_id_t chosen_id = ready_list_.front();
        ready_list_.pop_front();

        id_tcb_map_[chosen_id].setState(ThreadState::eRUNNING);
        running_thread_id_ = chosen_id;
        startFirstTask((uint32_t) (id_tcb_map_[chosen_id].sp()));
    }

    void switchCurrentThreadTo(const ThreadState& state)
    {
        // Voluntary context switch and 
        // involuntary context switch 
        // could generate race condition.
        // Therefore disable interrupts here.
        disable_interrupts();
        
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
        
        thread_switch(id_tcb_map_[prev_thread_id], id_tcb_map_[running_thread_id_]);
        enable_interrupts();
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


    void exit()
    {
        switchCurrentThreadTo(ThreadState::eFINISHED);
    }

    thread_id_t runningThreadID() const { return running_thread_id_; }

    const ecs::list<thread_id_t>& readyList() const { return ready_list_; }
private:
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
        disable_interrupts();
        if(isLocked())
        {
            ret = false;
        }else
        {
            owner_ = schedulerInstance().runningThreadID();
            ret = true;
        }
        enable_interrupts();
        return ret;
    }

    void lock()
    {
        disable_interrupts();
        if(isLocked())
        {
            if(schedulerInstance().runningThreadID() == owner_)
            {
                // lock twice
                // deal lock would occur
            }else
            {
                waiting_list_.push_back(schedulerInstance().runningThreadID());
                schedulerInstance().suspend();
            }
            
        }else
        {
            owner_ = schedulerInstance().runningThreadID();
        }
        enable_interrupts();
    }

    void unlock()
    {
        disable_interrupts();
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
            waiting_list_.pop_front();
            schedulerInstance().resume(owner_);
        }
        enable_interrupts();
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
    void lock(mutex_id_t id) { mtx_map_[id].lock(); }
    void try_lock(mutex_id_t id) { mtx_map_[id].try_lock(); }
    void unlock(mutex_id_t id) { mtx_map_[id].unlock(); }
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
    (*f)(arg);
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



} // namespace cs251

#endif // _CS251_OS_H
