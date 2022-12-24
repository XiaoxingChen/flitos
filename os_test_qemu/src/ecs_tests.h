#if !defined(_ECS_TESTS_H_)
#define _ECS_TESTS_H_

#include "ecs_unordered_map.h"
#include "ecs_allocator_nosys.h"
#include "uart_printf.h"

namespace ecs
{

inline int unorderedMapTest01()
{
    ecs::unordered_map<int, int, allocator_nosys<pair<int, int>>> m;
    m[0] = 0;
    m[2] = 2;
    m.insert(ecs::pair<int, int>(1,1));
    
    for(size_t i = 0; i < m.size(); i++)
    {
        // std::cout << i <<  ", " << m[i] <<std::endl;
        if(m[i] != int(i))
        {
            raw_printf("%d, %d\n", i, m[i]);
            raw_printf("size: %d\n", m.size());
            raw_printf("%s:%d\n", __FILE__, __LINE__);
            return 1;
        }
    }
#if 1
    if(m.count(0) != 1)
    {
        raw_printf("%s:%d\n", __FILE__, __LINE__);
        return 1;
    }

    if(m.count(3) != 0)
    {
        raw_printf("%s:%d\n", __FILE__, __LINE__);
        return 1;
    }

    m.erase(0);
    if(m.count(0) != 0)
    {
        raw_printf("%s:%d\n", __FILE__, __LINE__);
        return 1;
    }

    raw_printf("PASS %s()\n", __FUNCTION__);
    return 0;
#endif
}

inline int unorderedMapTest02()
{
    ecs::unordered_map<int, int, allocator_nosys<pair<int, int>>> m;
    for(int i = 0; i < 20; i++)
    {
        m[i] = i;
    }

    for(int i = 0; i < 20; i++)
    {
        m[i] += 1;
    }

    for(int i = 0; i < 20; i++)
    {
        if(m[i] != i+1)
        {
            raw_printf("%s:%d\n", __FILE__, __LINE__);
            return 1;
        }
    }
    raw_printf("PASS %s()\n", __FUNCTION__);
    return 0;
}

inline int unorderedMapTest()
{
    if(unorderedMapTest01() != 0) return 1;
    if(unorderedMapTest02() != 0) return 1;
    return 0;
}

inline int runFullTests()
{
    if(unorderedMapTest() != 0) return 1;
    return 0;
}

} // namespace ecs



#endif // _ECS_TESTS_H_
