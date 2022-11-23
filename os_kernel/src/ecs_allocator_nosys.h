#if !defined(_ECS_ALLOCATOR_NOSYS_H_)
#define _ECS_ALLOCATOR_NOSYS_H_
#include <stdlib.h>

namespace ecs
{
    
template<typename T>
class allocator_nosys
{
public:
    T* allocate(size_t n)
    {
        return static_cast<T*>(malloc(n * sizeof(T)));
    }
    void deallocate(T* p, size_t n)
    {
        free(p);
    }
private:

};
} // namespace ecs

#endif // _ECS_ALLOCATOR_NOSYS_H_
