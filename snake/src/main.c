#include <stdint.h>
#include <stdlib.h>
// #include "snake.h"
#include "cs251_os.h"
#include <stddef.h>
volatile int global = 42;
volatile uint32_t controller_status = 0;
// extern "C" void context_switch(volatile size_t** oldsp, volatile size_t* newsp);

void thread_fun(void* arg)
{
    while(1);
}

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

void idleThread(void* param)
{
    uint32_t offset = *(uint32_t*)param;
    int cnt = 0;
    for(int i = 0; i < 1000;)
    {
        VIDEO_MEMORY[offset] = '0' + cnt++ % 10;
        cs251::thread_yield();
    }
}

// #define IDLE_THREAD_STK_SIZE (0x1000)
// #define SIZE_OF_POPAD (13)
// uint8_t idleThreadStack[IDLE_THREAD_STK_SIZE];

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

namespace cs251
{
    void* g_scheduler_ = nullptr;
} // namespace cs251
// ecs::map<int, int> a;

int main() {
    int last_global = 42;
    
    // memset(idleThreadStack, 0, IDLE_THREAD_STK_SIZE);
    // for(int i = 0; i < IDLE_THREAD_STK_SIZE; i++)
    // {
    //     idleThreadStack[i] = i & 0xff;
    // }
    // initForIdleThread();
    uint32_t display_offsets[] = {5,6,7};

    // scheduler.clearFinishedList();
    cs251::schedulerInstance().create(idleThread, &display_offsets[0]);
    cs251::schedulerInstance().create(idleThread, &display_offsets[1]);
    cs251::schedulerInstance().create(idleThread, &display_offsets[2]);
    
    cs251::schedulerInstance().launchFirstTask();

    while (1) ;
    
    return 0;
}
