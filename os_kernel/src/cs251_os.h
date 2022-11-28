#if !defined(_CS251_OS_H)
#define _CS251_OS_H

#include <stdlib.h>
#include <stdint.h>
#include "ecs_list.h"
#include "ecs_queue.h"
#include "ecs_map.h"
#include "port_riscv.h"
#include "ecs_assert.h"
#include "ecs_allocator_nosys.h"

#if defined(VIRT_CLINT)
#include "uart_printf.h"
#else
#define LOGD(...) while(0)
#endif // VIRT_CLINT


extern "C" void context_switch(volatile size_t** oldsp, volatile size_t* newsp);
extern "C" void disable_interrupts();
extern "C" void enable_interrupts();
extern "C" void startFirstTask( uint32_t stk_ptr );
extern "C" uint32_t global_ptr_read();
extern "C" void global_ptr_write(uint32_t stk_ptr);



namespace cs251
{
enum ThreadState
{
    eINIT = 0,
    eREADY,
    eRUNNING,
    eWAITING,
    eFINISHED,
    eUNKNOWN
};

// constexpr size_t SIZE_OF_POPAD = 14;
constexpr size_t INITIAL_STACK_SIZE = 0x1000;
using thread_id_t = int;
using mutex_id_t = int;
using cond_id_t = int;

class ThreadControlBlock;
class ThreadScheduler;
class MutexFactory;
class ConditionVariableFactory;

void stub_wrapper(void (*f)(void*), void* arg, size_t thread_global_ptr);
void thread_switch(ThreadControlBlock& curr_tcb, ThreadControlBlock& next_tcb);
inline ThreadScheduler& schedulerInstance();
inline MutexFactory& mutexFactoryInstance();
inline ConditionVariableFactory& condFactoryInstance();


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
        *(stack_ptr_ + static_cast<size_t>(offset_gp)) = global_ptr_read() ; // use os global pointer to context
        *(stack_ptr_ + static_cast<size_t>(offset_mstatus)) = 0x1880; //make sure interrupt is closed when context switch finished.
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
    void init(void (*f)(void*), void* arg, size_t thread_global_ptr);
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
    cond_id_t condForJoin() const { return cond_for_join_; }
    void setPreemption(bool on)
    {
        if(on) preemption_on_ ++;
        else preemption_on_--;
        if(preemption_on_ > 1)
            assert(false);
    }
    bool allowPreemption() const
    {
        return preemption_on_ > 0;
    }

private:
    
    // thread_id_t thread_id_ = 0;
    size_t stk_size_ = 0;
    uint8_t* stk_mem_ = nullptr;
    size_t* stack_ptr_ = nullptr;
    void* program_counter_ = nullptr;
    int preemption_on_ = 1;
    ThreadState state_ = eINIT;
    mutex_id_t mtx_for_join_ = -1;
    cond_id_t cond_for_join_ = -1;
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

    thread_id_t create(void (*f)(void*), void* arg, size_t thread_global_ptr)
    {
        thread_counter_++;
        thread_id_t id = thread_counter_;
        id_tcb_map_[id] = ThreadControlBlock();
        id_tcb_map_[id].init(f, arg, thread_global_ptr);
        ready_list_.push_back(id);
        LOGD("create thread: %d\n", id);
        return id;
    }

    thread_id_t create(void (*f)(void*), void* arg)
    {
        size_t thread_global_ptr = global_ptr_read();
        return create(f, arg, thread_global_ptr);
    }


    void launchFirstTask()
    {
        if(ready_list_.empty()) return; // idle thread should always be in ready_list?
        thread_id_t chosen_id = ready_list_.front();
        ready_list_.pop_front();

        id_tcb_map_[chosen_id].setState(ThreadState::eRUNNING);
        running_thread_id_ = chosen_id;
        LOGD("launch first task: %d\n", chosen_id);
        startFirstTask((uint32_t) (id_tcb_map_[chosen_id].sp()));
    }

    void switchCurrentThreadTo(const ThreadState& state)
    {
        // Voluntary context switch and 
        // involuntary context switch 
        // could generate race condition.
        // Therefore disable preemption here.
        setPreemption(false);
        do
        {
            if(state != ThreadState::eREADY 
            && state != ThreadState::eWAITING
            && state != ThreadState::eFINISHED) break;
            
            if(ready_list_.empty()) break; // running thread is idel spin thread

            thread_id_t curr_thread_id = running_thread_id_;
            thread_id_t next_thread_id = ready_list_.front();
            ready_list_.pop_front();
            assert(curr_thread_id != next_thread_id);
            // LOGD("%s:%d\n", __FILE__, __LINE__);

            id_tcb_map_[curr_thread_id].setState(state);
            id_tcb_map_[next_thread_id].setState(ThreadState::eRUNNING);

            if(state == ThreadState::eREADY)
            {
                ready_list_.push_back(curr_thread_id);
            }else if (state == ThreadState::eFINISHED)
            {
                finished_list_.push_back(curr_thread_id);
            }
            
            debugContextSwitchInfo(curr_thread_id, next_thread_id, false);

            disable_interrupts();
            running_thread_id_ = next_thread_id;
            thread_switch(id_tcb_map_[curr_thread_id], id_tcb_map_[next_thread_id]);
            enable_interrupts();
        } while (0);
        // same logic as [preemption context]
        setPreemption(true);
    }

    void debugContextSwitchInfo(thread_id_t curr_thread_id, thread_id_t next_thread_id, bool preemption)
    {
        assert(curr_thread_id != next_thread_id);
        
        if(preemption)
        {
            LOGD("Preemptive ");
        }else
        {
            LOGD("Voluntary ");
            if(allowPreemption())
                assert(false);
        }
        LOGD("context switch from %d to %d. ready_list_: ", curr_thread_id, next_thread_id);
        for(size_t i = 0; i < ready_list_.size(); i++)
        {
            LOGD("%d ", ready_list_.at(i));
        }LOGD("\n");
    }

    void inInterruptYield()
    {
        if(! allowPreemption())
            return;
        if(ready_list_.empty()) return; // running thread is idel spin thread

        thread_id_t curr_thread_id = running_thread_id_;
        thread_id_t next_thread_id = ready_list_.front();
        ready_list_.pop_front();
        
        id_tcb_map_[curr_thread_id].setState(ThreadState::eREADY);
        id_tcb_map_[next_thread_id].setState(ThreadState::eRUNNING);

        ready_list_.push_back(curr_thread_id);
        debugContextSwitchInfo(curr_thread_id, next_thread_id, true);
        
        running_thread_id_ = next_thread_id;
        thread_switch(id_tcb_map_[curr_thread_id], id_tcb_map_[next_thread_id]);
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

    void setPreemption(bool on) 
    {
        #if 0
            disable_interrupts();
            if(on) preemption_on_ ++;
            else preemption_on_--;
            // LOGD("thread %d set preemtion to %d\n",runningThreadID(), preemption_on_); 
            if(preemption_on_ > 1)
                assert(false);
            enable_interrupts();
        #else
            id_tcb_map_[running_thread_id_].setPreemption(on);
        #endif
    }

    bool allowPreemption() const 
    { 
        if(id_tcb_map_.size() == 0) return false;
        return id_tcb_map_.at(running_thread_id_).allowPreemption(); 
    }

    
    void exit()
    {
        switchCurrentThreadTo(ThreadState::eFINISHED);
    }

    ThreadState threadState( thread_id_t tid ) const
    {
        if(tid > thread_counter_) return ThreadState::eUNKNOWN;
        auto it = id_tcb_map_.find(tid);
        if(it == id_tcb_map_.end()) return ThreadState::eFINISHED;
        return it->second.state();
    }

    thread_id_t runningThreadID() const { return running_thread_id_; }

    const ecs::deque<thread_id_t, ecs::allocator_nosys<thread_id_t>>& 
    readyList() const { return ready_list_; }
    const ecs::map<thread_id_t, ThreadControlBlock>& idTcbMap() const { return id_tcb_map_; }
private:
    // volatile int preemption_on_ = 0;
    thread_id_t running_thread_id_ = 0;
    size_t thread_counter_ = 0;
    ecs::deque<thread_id_t, ecs::allocator_nosys<thread_id_t>> ready_list_;
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

class SleepTimer
{
public:
    SleepTimer()
    {
        mtx_for_sleep_ = mutexFactoryInstance().create();
        cond_for_sleep_ = condFactoryInstance().create();
    }
    void sleep(size_t cnt)
    {
        size_t wake_tick = systick_ + cnt;
        mutexFactoryInstance().lock(mtx_for_sleep_);
        
        while(systick_ < wake_tick)
        {
            condFactoryInstance().wait(cond_for_sleep_, mtx_for_sleep_);
        }
        mutexFactoryInstance().unlock(mtx_for_sleep_);
    }

    void updateTick()
    { 
        systick_++; 
        condFactoryInstance().notifyAll(cond_for_sleep_);
    }

private:
    volatile size_t systick_ = 0;
    mutex_id_t mtx_for_sleep_ = -1;
    cond_id_t cond_for_sleep_ = -1;
};

extern void* g_mutex_factory;
extern void* g_condition_variable_factory;
extern SleepTimer* g_sleep_timer;

inline SleepTimer& sleepTimerInstance()
{
    if(g_sleep_timer == nullptr)
        g_sleep_timer = new SleepTimer();
    return *g_sleep_timer;
}

inline MutexFactory& mutexFactoryInstance()
{
    if(g_mutex_factory == nullptr)
        g_mutex_factory = (void*) new MutexFactory();
    return *(MutexFactory*)g_mutex_factory;
}

inline ConditionVariableFactory& condFactoryInstance()
{
    if(g_condition_variable_factory == nullptr)
        g_condition_variable_factory = (void*) new ConditionVariableFactory();
    return *(ConditionVariableFactory*)g_condition_variable_factory;
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

inline void stub_wrapper(void (*f)(void*), void* arg, size_t thread_global_ptr)
{
    // increaseTimeCompare(1000);
    size_t os_global_ptr = global_ptr_read();
    enable_interrupts();
    global_ptr_write(thread_global_ptr);
    (*f)(arg);
    global_ptr_write(os_global_ptr);
    
    cond_id_t cond_for_join = schedulerInstance().idTcbMap().at(schedulerInstance().runningThreadID()).condForJoin();
    condFactoryInstance().notifyAll( cond_for_join );
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

inline void ThreadControlBlock::init(void (*f)(void*), void* arg, size_t thread_global_ptr)
{
    // thread_id_ = threadCounter();
    stk_size_ = INITIAL_STACK_SIZE;
    stk_mem_ = (uint8_t*)malloc(INITIAL_STACK_SIZE);
    stack_ptr_ = reinterpret_cast<size_t*>(stk_mem_ + INITIAL_STACK_SIZE) - 1; //todo
    program_counter_ = reinterpret_cast<void*>(stub_wrapper);

    pushDummySwitchFrame();
    *(stack_ptr_ + static_cast<size_t>(offset_a2)) = thread_global_ptr;
    *(stack_ptr_ + static_cast<size_t>(offset_a1)) = reinterpret_cast<size_t>(arg);
    *(stack_ptr_ + static_cast<size_t>(offset_a0)) = reinterpret_cast<size_t>(f);
    state_ = ThreadState::eREADY;

    
    mtx_for_join_ = mutexFactoryInstance().create();
    cond_for_join_ = condFactoryInstance().create();

    preemption_on_ = 1;
}

inline void ThreadScheduler::join( thread_id_t tid )
{
    mutex_id_t mtx_for_join = id_tcb_map_[tid].mutexForJoin();
    cond_id_t cond_for_join = id_tcb_map_[tid].condForJoin();
    mutexFactoryInstance().lock(mtx_for_join);
    assert(threadState(tid) != ThreadState::eUNKNOWN);
    while(threadState(tid) != ThreadState::eFINISHED)
    {
        condFactoryInstance().wait(cond_for_join, mtx_for_join);
    }
    mutexFactoryInstance().unlock(mtx_for_join);
}

#ifdef CS251_OS_STATIC_OBJECTS_ON
void* g_scheduler_ = nullptr;
void* g_mutex_factory = nullptr;
void* g_condition_variable_factory = nullptr;
SleepTimer* g_sleep_timer = nullptr;
#endif

} // namespace cs251

#endif // _CS251_OS_H
