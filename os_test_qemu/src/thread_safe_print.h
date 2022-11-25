#if !defined(_THREAD_SAFE_PRINTF_H_)
#define _THREAD_SAFE_PRINTF_H_

#include "cs251_os.h"
#include "ecs_vector.h"
#include "uart_printf.h"
#include "cs251_thread_safe_queue.h"

namespace cs251
{

void threadConsoleRunner(void* param);

extern void* p_console_queue;

// using TypeConsoleContent = ecs::string;
using TypeConsoleContent = char;

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
        p_que->dequeue([UART_MEMORY](const char& c){UART_MEMORY[0] = c;}, p_que->size());
    }
}

#endif
} // namespace cs251

#ifdef THREAD_SAFE_PRINT_IMPLEMENTATION

#else
int printf(const char *format, ...);

#endif

#endif // _THREAD_SAFE_PRINTF_H_
