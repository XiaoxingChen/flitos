#if !defined(_THREAD_API_H_)
#define _THREAD_API_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef int thread_id_t;
typedef int mutex_id_t;

thread_id_t threadCreate(void (*f)(void*), void* arg);
uint32_t threadYield();
void threadJoin(thread_id_t tid);
mutex_id_t mutexInit();
void mutexDestroy(mutex_id_t mtx_id);
void mutexLock(mutex_id_t mtx_id);
void mutexUnlock(mutex_id_t mtx_id);
void threadSleep(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif // _THREAD_API_H_
