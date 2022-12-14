#if !defined(_CS251_SCHEDULER_H_)
#define _CS251_SCHEDULER_H_

#include <stdlib.h>
#include <stdint.h>
#include "ecs_list.h"
#include "ecs_queue.h"
#include "ecs_map.h"
#include "ecs_unordered_map.h"
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
class SleepTimer;
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
        setPreemption(false);
        thread_counter_++;
        thread_id_t id = thread_counter_;
        id_tcb_map_[id] = ThreadControlBlock();
        id_tcb_map_[id].init(f, arg, thread_global_ptr);
        ready_list_.push_back(id);
        LOGD("create thread: %d\n", id);
        setPreemption(true);
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
        if(size_t(tid) > thread_counter_) return ThreadState::eUNKNOWN;
        auto it = id_tcb_map_.find(tid);
        if(it == id_tcb_map_.end()) return ThreadState::eFINISHED;
        return it->second.state();
    }

    thread_id_t runningThreadID() const { return running_thread_id_; }

    const ecs::deque<thread_id_t, ecs::allocator_nosys<thread_id_t>>& 
    readyList() const { return ready_list_; }

    using IdTcbMapType = ecs::unordered_map<thread_id_t, ThreadControlBlock, ecs::allocator_nosys<ecs::pair<thread_id_t, ThreadControlBlock>>>;
    // using IdTcbMapType = ecs::map<thread_id_t, ThreadControlBlock>;

    const IdTcbMapType& idTcbMap() const { return id_tcb_map_; }
private:
    // volatile int preemption_on_ = 0;
    thread_id_t running_thread_id_ = 0;
    size_t thread_counter_ = 0;
    ecs::deque<thread_id_t, ecs::allocator_nosys<thread_id_t>> ready_list_;
    // ecs::list<thread_id_t> waiting_list_;
    ecs::list<thread_id_t> finished_list_;
    IdTcbMapType id_tcb_map_;
};


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

#ifdef CS251_OS_STATIC_OBJECTS_ON
void* g_scheduler_ = nullptr;
void* g_mutex_factory = nullptr;
void* g_condition_variable_factory = nullptr;
SleepTimer* g_sleep_timer = nullptr;
#endif

} // namespace cs251


#endif // _CS251_SCHEDULER_H_
