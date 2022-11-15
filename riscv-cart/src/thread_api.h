#if !defined(_THREAD_API_H_)
#define _THREAD_API_H_
#include <stdint.h>

uint32_t threadCreate(void (*f)(void*), void* arg);
uint32_t threadYield();

#endif // _THREAD_API_H_
