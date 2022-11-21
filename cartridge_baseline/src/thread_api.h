#if !defined(_THREAD_API_H_)
#define _THREAD_API_H_
#include <stdint.h>

typedef int thread_id_t;
typedef int mutex_id_t;

thread_id_t threadCreate(void (*f)(void*), void* arg);
uint32_t threadYield();
void threadJoin(thread_id_t tid);
mutex_id_t mutexInit();
void mutexDestroy(mutex_id_t mtx_id);
void mutexAcquire(mutex_id_t mtx_id);
void mutexRelease(mutex_id_t mtx_id);

#endif // _THREAD_API_H_
