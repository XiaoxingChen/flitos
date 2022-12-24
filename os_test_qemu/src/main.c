#include <stdint.h>
#include <stdlib.h>
#include "uart_printf.h"
#include "ecs_string.h"

#define CS251_OS_STATIC_OBJECTS_ON
#define ALLOCATOR_IMPLEMENTATION
#include "ecs_allocator.h"
#undef ALLOCATOR_IMPLEMENTATION
#include "cs251_pipe.h"



#include <stddef.h>
// #include "thread_api.h"

#define THREAD_SAFE_PRINT_IMPLEMENTATION
#include "thread_safe_print.h"

volatile int global = 42;
volatile uint32_t controller_status = 0;
// extern "C" void context_switch(volatile size_t** oldsp, volatile size_t* newsp);

void thread_fun(void* arg)
{
    while(1);
}

// volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

char VIDEO_MEMORY[0x40 * 30];
char * UART_MEMORY = (char*)(0x10000000);

void idleThread(void* param)
{
    // uint32_t offset = *(uint32_t*)param;
    int cnt = 0;
    for(int i = 0; i < 1000;)
    {
        cs251::thread_yield();
    }
}

struct MutexCount
{
    int mtx_counter;
    int mtx_finish;
    int cond_counter;
    int* p_counter;
    int* p_finish_cnt;
};

void naiveThread(void* param)
{
    uint32_t offset = *(uint32_t*)param;
    int cnt = 0;
    for(int i = 0; i < 1000;i++)
    {
        VIDEO_MEMORY[offset] = '0' + cnt++ % 10;
        // cs251::thread_yield();
    }
    VIDEO_MEMORY[offset] = '#';
}

void mutexVerifyThread(void* args)
{
    MutexCount* p_mtx_cnt = (MutexCount*)args;
    for(int i = 0; i < 1000; i++)
    {
        cs251::mutexFactoryInstance().lock(p_mtx_cnt->mtx_counter);
        *(p_mtx_cnt->p_counter) += 1;
        cs251::mutexFactoryInstance().unlock(p_mtx_cnt->mtx_counter);
        cs251::condFactoryInstance().notifyOne(p_mtx_cnt->cond_counter);
        // cs251::thread_yield();
    }
    *(p_mtx_cnt->p_finish_cnt) += 1;
}

void displayThread(void* args)
{
    MutexCount* p_mtx_cnt = (MutexCount*)args;
    int val = 0; 
    int last_val = 0;

    // threadJoin(3);
    // cs251::schedulerInstance().join(3);

    
    while(1)
    {
        cs251::mutexFactoryInstance().lock(p_mtx_cnt->mtx_counter);
        while(*(p_mtx_cnt->p_counter) == last_val)
        {
            cs251::condFactoryInstance().wait(p_mtx_cnt->cond_counter, p_mtx_cnt->mtx_counter);
        }
        val = *(p_mtx_cnt->p_counter);
        cs251::mutexFactoryInstance().unlock(p_mtx_cnt->mtx_counter);
        {
            printf("cnt: %d\n", val);
            last_val = val;
            // cs251::consoleQueueInstance().enqueue(ecs::string("t"));
            // cs251::consoleQueueInstance().enqueue('a');
        }
        // cs251::thread_yield();
    }
}

void threadPipeWrite(void* param)
{
    int pipe_id = *(int*)param;
    ecs::string msg("hello baby\n");
    while(1)
    {
        cs251::sleepTimerInstance().sleep(400);
        // threadSleep(400);
        cs251::pipeFactoryInstance().write(pipe_id, (uint8_t*)msg.c_str(), msg.size());
    }
}

void threadPipeRead(void* param)
{
    int pipe_id = *(int*)param;
    char buff[100];
    while(1)
    {
        size_t n = cs251::pipeFactoryInstance().read(pipe_id, (uint8_t*)buff, 100);
        buff[n] = '\x00';
        printf("%s", buff);
    }
}

extern "C" void startFirstTask( uint32_t stk_ptr );
#if 0
void initForIdleThread()
{
    uint32_t* stack_ptr_ = (uint32_t*)(idleThreadStack + IDLE_THREAD_STK_SIZE) - 1;    
    stack_ptr_ -= SIZE_OF_POPAD;
    *(stack_ptr_ + 12) = (uint32_t)&idleThread; // ra
    *(stack_ptr_ + 5) = 5; // a0
    startFirstTask((uint32_t)stack_ptr_);
}
#endif
void increaseTimeCompare(uint32_t val);

void threadTestJoin1(void*)
{
    int last_global = global;
    
    for(int i = 0; i < 10; i++)
    {
        cs251::sleepTimerInstance().sleep(400);
        // threadSleep(400);
        printf("join 1 cnt: %d\n", i);
    }
    

    printf("finish thread test join1\n");
}

void threadTestJoin2(void*)
{
    auto th_join_1 = cs251::schedulerInstance().create(threadTestJoin1, nullptr);
    cs251::schedulerInstance().join(th_join_1);
    // threadJoin(th_join_1);
    printf("finish thread test join2\n");
}

void threadMain(void*)
{
    cs251::initConsoleThread();
    
    ecs::vector<cs251::thread_id_t> threads_to_join;
    int pipe_id = cs251::pipeFactoryInstance().open();
    // threads_to_join.push_back(cs251::schedulerInstance().create(threadPipeRead, &pipe_id));
    // threads_to_join.push_back(cs251::schedulerInstance().create(threadPipeWrite, &pipe_id));

    // threads_to_join.push_back(cs251::schedulerInstance().create(threadTestJoin2, nullptr));

    printf("main thread\n");

    for(size_t i = 0; i < threads_to_join.size(); i++)
    {
        cs251::schedulerInstance().join(threads_to_join.at(i));
    }

    printf("test finish\n");

    uint32_t exit_code = 0;
    uint32_t * VIRT_TEST = (uint32_t*)(0x10'0000);
    VIRT_TEST[0] = (exit_code << 16) | 0x3333;
    // VIRT_TEST[0] = 0x7777;
    return;
}



int main() {
    
    raw_printf("\n\n\n\n\n\n\n\n=== CS251 OS START! === \n\n\n\n\n\n\n\n");

    disable_interrupts();
    
    cs251::schedulerInstance().create(idleThread, nullptr);
    cs251::schedulerInstance().create(threadMain, nullptr);
    
    cs251::schedulerInstance().launchFirstTask();
    
    return 0;
}
