#if !defined(_THREAD_SAFE_PRINTF_H_)
#define _THREAD_SAFE_PRINTF_H_

#include "cs251_os.h"
#include "ecs_vector.h"
#include "ecs_string.h"
#include "uart_printf.h"
#include "ecs_allocator.h"
#include "ecs_queue.h"

namespace cs251
{
    

template<typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue()
    {
        mtx_que_ = cs251::mutexFactoryInstance().create();
        cond_que_ = cs251::condFactoryInstance().create();
    }
    void enqueue(T&& val)
    {
        cs251::mutexFactoryInstance().lock(mtx_que_);
#if 1
        q_.push_back(val);
#else
        cnt_ ++;
#endif

        cs251::mutexFactoryInstance().unlock(mtx_que_);
        cs251::condFactoryInstance().notifyOne(cond_que_);
    }

    T dequeue()
    {
        cs251::mutexFactoryInstance().lock(mtx_que_);
        while(empty())
        {
            raw_printf("dequeue\n");
            cs251::condFactoryInstance().wait(cond_que_, mtx_que_);
        }
#if 1
        T ret = q_.front();
        q_.pop_front();
#else
        cnt_ -= 1;
#endif
        cs251::mutexFactoryInstance().unlock(mtx_que_);
        return ret;
    }

    size_t size() const
    {
        return q_.size();
    }
    bool empty() const
    {
        return q_.empty();
    }

private:
    size_t cnt_;
    cs251::mutex_id_t mtx_que_;
    cs251::cond_id_t cond_que_;
    ecs::deque<T> q_;
};


void threadConsoleRunner(void* param);




extern void* p_console_queue;

// using TypeConsoleContent = ecs::string;
using TypeConsoleContent = ecs::vector<char>;

inline ThreadSafeQueue<TypeConsoleContent>& consoleQueueInstance()
{
    if(p_console_queue == nullptr)
    {
        p_console_queue = new ThreadSafeQueue<TypeConsoleContent>();
    }
    return *static_cast<ThreadSafeQueue<TypeConsoleContent>*>(p_console_queue);
}

inline void initConsoleThread()
{
    cs251::schedulerInstance().create(threadConsoleRunner, &consoleQueueInstance());
}

#ifdef THREAD_SAFE_PRINT_IMPLEMENTATION

void* p_console_queue = nullptr;

void threadConsoleRunner(void* param)
{
    ThreadSafeQueue<TypeConsoleContent>* p_que = static_cast<ThreadSafeQueue<TypeConsoleContent>*>(param);
    char * UART_MEMORY = (char*)(0x10000000);
    
    
    while(1)
    {
        TypeConsoleContent msg;
        msg = p_que->dequeue();
        // UART_MEMORY[0] = msg;
        for(int i = 0; i < msg.size(); i++)
        {
            UART_MEMORY[0] = msg.at(i);
        }
    }
}

#endif
} // namespace cs251

#ifdef THREAD_SAFE_PRINT_IMPLEMENTATION

#else
int printf(const char *format, ...);

#endif

#endif // _THREAD_SAFE_PRINTF_H_
