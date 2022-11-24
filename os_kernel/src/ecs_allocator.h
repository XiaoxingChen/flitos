#if !defined(_ECS_ALLOCATOR_H_)
#define _ECS_ALLOCATOR_H_

#include "cs251_os.h"

namespace ecs
{
#ifdef ALLOCATOR_IMPLEMENTATION
int g_mtx_allocator = -1;

cs251::mutex_id_t allocatorMutexInstance()
{
    if(g_mtx_allocator == -1)
    {
        g_mtx_allocator = cs251::mutexFactoryInstance().create();
    }
    return g_mtx_allocator;
}
#endif
cs251::mutex_id_t allocatorMutexInstance();



template<typename T>
class allocator
{
public:
    T* allocate(size_t n)
    {
        T* ret = nullptr;
        cs251::mutex_id_t mtx = allocatorMutexInstance();
        cs251::mutexFactoryInstance().lock(mtx);
        // uint32_t return_address;
        // asm volatile ("mv %0, ra" : "=r"(return_address));
        // LOGD("%s:%d; thread: %d; ra: 0x%X,", __FILE__, __LINE__, cs251::schedulerInstance().runningThreadID(), return_address);
        ret = static_cast<T*>(malloc(n * sizeof(T)));
        // LOGD("addr: 0x%X\n", ret);
        cs251::mutexFactoryInstance().unlock(mtx);
        return ret;
    }
    void deallocate(T* p, size_t n)
    {
        cs251::mutex_id_t mtx = allocatorMutexInstance();
        cs251::mutexFactoryInstance().lock(mtx);
        // uint32_t return_address;
        // asm volatile ("mv %0, ra" : "=r"(return_address));
        // LOGD("%s:%d; thread: %d; ra: 0x%X\n", __FILE__, __LINE__, cs251::schedulerInstance().runningThreadID(), return_address);
        free(p);
        cs251::mutexFactoryInstance().unlock(mtx);
    }
private:
};
} // namespace ecs

#endif // _ECS_ALLOCATOR_H_
