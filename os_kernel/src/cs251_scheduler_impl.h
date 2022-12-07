#if !defined(_CS251_SCHEDULER_IMPL_H_)
#define _CS251_SCHEDULER_IMPL_H_

#include "cs251_condition_variable.h"
#include "cs251_mutex.h"

namespace cs251
{

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


} // namespace cs251

#endif // _CS251_SCHEDULER_IMPL_H_
