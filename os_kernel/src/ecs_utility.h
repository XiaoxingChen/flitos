#if !defined(_ECS_UTILITY_H_)
#define _ECS_UTILITY_H_

namespace ecs
{

template<typename TKey, typename TValue>
struct pair{
    pair(const TKey& key, const TValue& val): first(key), second(val){}
    pair() = default;
    TKey first;
    TValue second;
};

} // namespace ecs



#endif // _ECS_UTILITY_H_
