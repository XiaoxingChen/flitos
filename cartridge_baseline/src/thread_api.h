#if !defined(_THREAD_API_H_)
#define _THREAD_API_H_
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef int thread_id_t;
typedef int mutex_id_t;
typedef int cond_id_t;
typedef int pipe_id_t;

// thread
thread_id_t threadCreate(void (*f)(void*), void* arg);
uint32_t threadYield();
void threadJoin(thread_id_t tid);
void threadSleep(uint32_t ms);

// mutex
mutex_id_t mutexInit();
void mutexDestroy(mutex_id_t mtx_id);
void mutexLock(mutex_id_t mtx_id);
void mutexUnlock(mutex_id_t mtx_id);

// condition variable
cond_id_t condInit();
void condDestroy(cond_id_t cond_id);
void condSignal(cond_id_t cond_id);
void condBroadcast(cond_id_t cond_id);
void condWait(cond_id_t id, mutex_id_t mtx_id);

// pipe: inter-thread communication
pipe_id_t pipeOpen();
void pipeClose(pipe_id_t id);
int pipeRead(pipe_id_t id, uint8_t* buff, int len);
void pipeWrite(pipe_id_t id, uint8_t* buff, int len);

// thread-safe malloc free
void* malloc(int size);
void free(void* ptr);



#ifdef __cplusplus
}
#endif

#endif // _THREAD_API_H_
