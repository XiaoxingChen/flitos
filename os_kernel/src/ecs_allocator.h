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
        ret = static_cast<T*>(malloc(n * sizeof(T)));
        cs251::mutexFactoryInstance().unlock(mtx);
        return ret;
    }
    void deallocate(T* p, size_t n)
    {
        cs251::mutex_id_t mtx = allocatorMutexInstance();
        cs251::mutexFactoryInstance().lock(mtx);
        free(p);
        cs251::mutexFactoryInstance().unlock(mtx);
    }
private:
};
} // namespace ecs

#endif // _ECS_ALLOCATOR_H_
