#include "thread_api.h"

int threadCreateRaw(uint32_t f, uint32_t arg, uint32_t global_pointer);

thread_id_t threadCreate(void (*f)(void*), void* arg)
{
    uint32_t thread_global_pointer;
    asm volatile ("mv %0, gp" : "=r"(thread_global_pointer));
    threadCreateRaw((uint32_t)f, (uint32_t)arg, thread_global_pointer);
}