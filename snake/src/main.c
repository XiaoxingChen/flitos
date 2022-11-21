#include <stdint.h>
#include <stdlib.h>
// #include "snake.h"
#include "cs251_os.h"
#include <stddef.h>
#include "nanoprintf.h"
volatile int global = 42;
volatile uint32_t controller_status = 0;
// extern "C" void context_switch(volatile size_t** oldsp, volatile size_t* newsp);

void thread_fun(void* arg)
{
    while(1);
}

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

#if 0
int line_printf(int idx, const char *format, ...)
{
    char *VIDEO_MEMORY = (char *)(0x50000000 + 0xFE800);
    unsigned int buff_len = 0x40;
    va_list args;
    int n = buff_len;
    va_start(args, format);
	n = npf_vsnprintf(&VIDEO_MEMORY[0x40 * idx], buff_len, format, args);
	va_end(args);

    return n;
}
#endif

// char VIDEO_MEMORY[0x40 * 30];

void idleThread(void* param)
{
    uint32_t offset = *(uint32_t*)param;
    int cnt = 0;
    for(int i = 0; i < 1000;)
    {
        // VIDEO_MEMORY[offset] = '0' + cnt++ % 10;
        line_printf(offset, "%d", cnt++);
        cs251::thread_yield();
    }
}

struct MutexCount
{
    int mtx_handle;
    int* p_counter;
};

void naiveThread(void* param)
{
    uint32_t offset = *(uint32_t*)param;
    int cnt = 0;
    for(int i = 0; i < 1000;i++)
    {
        VIDEO_MEMORY[offset] = '0' + cnt++ % 10;
    }
    VIDEO_MEMORY[offset] = '#';
}

void mutexVerifyThread(void* args)
{
    MutexCount* p_mtx_cnt = (MutexCount*)args;
    for(int i = 0; i < 500; i++)
    {
        cs251::mutexFactoryInstance().lock(p_mtx_cnt->mtx_handle);
        *(p_mtx_cnt->p_counter) += 1;
        cs251::mutexFactoryInstance().unlock(p_mtx_cnt->mtx_handle);
        cs251::thread_yield();
    }
    VIDEO_MEMORY[0x40 * 4] += 1;
}

void displayThread(void* args)
{
    MutexCount* p_mtx_cnt = (MutexCount*)args;
    int val = 0; 
    while(1)
    {
        cs251::mutexFactoryInstance().lock(p_mtx_cnt->mtx_handle);
        val = *(p_mtx_cnt->p_counter);
        cs251::mutexFactoryInstance().unlock(p_mtx_cnt->mtx_handle);
        line_printf(2, "shared val: %d", val);
        // VIDEO_MEMORY[0x40 * 2 + 5] = '0' + cs251::schedulerInstance().readyList().size();
        // VIDEO_MEMORY[0x40 * 0 + 1] = cs251::schedulerInstance().runningThreadID();
        // int i = 0;
        // for(i = 0; i < 0x40; i++) VIDEO_MEMORY[0x40 * 0 + 2 + i] = 0;
        // i = 0;
        // for(auto it = cs251::schedulerInstance().readyList().begin(); 
        // it != cs251::schedulerInstance().readyList().end(); 
        // it++, i++)
        // {
        //     VIDEO_MEMORY[0x40 * 0 + 2 + i] = *it;
        // }
        cs251::thread_yield();
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

namespace cs251
{
    void* g_scheduler_ = nullptr;
    void* g_mutex_factory = nullptr;
} // namespace cs251
// ecs::map<int, int> a;

int main() {
    cs251::g_scheduler_ = nullptr;
    cs251::g_mutex_factory = nullptr;
    int last_global = 42;
    
    // memset(idleThreadStack, 0, IDLE_THREAD_STK_SIZE);
    // for(int i = 0; i < IDLE_THREAD_STK_SIZE; i++)
    // {
    //     idleThreadStack[i] = i & 0xff;
    // }
    // initForIdleThread();
    VIDEO_MEMORY[0x40 * 4] = '0';
    uint32_t display_offsets[] = {0,1,0x40+2,0x40+3};

    MutexCount mtx_cnt;
    
    int cnt_var = 0;
    mtx_cnt.p_counter = &cnt_var;
    // mtx_cnt.p_counter = (int*)VIDEO_MEMORY;
    mtx_cnt.mtx_handle = cs251::mutexFactoryInstance().create();

    disable_interrupts();
    // scheduler.clearFinishedList();
    cs251::schedulerInstance().create(idleThread, &display_offsets[0]);
    cs251::schedulerInstance().create(idleThread, &display_offsets[1]);
    // cs251::schedulerInstance().create(naiveThread, &display_offsets[2]);
    // cs251::schedulerInstance().create(naiveThread, &display_offsets[3]);
    cs251::schedulerInstance().create(mutexVerifyThread, &mtx_cnt);
    cs251::schedulerInstance().create(mutexVerifyThread, &mtx_cnt);
    cs251::schedulerInstance().create(mutexVerifyThread, &mtx_cnt);
    cs251::schedulerInstance().create(displayThread, &mtx_cnt);
    
    cs251::schedulerInstance().launchFirstTask();

    while (1) ;
    
    return 0;
}
