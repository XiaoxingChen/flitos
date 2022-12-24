#if !defined(_ECS_ALGORITHM_H_)
#define _ECS_ALGORITHM_H_

namespace ecs
{
template<typename UnaryPredicate, class InputIt>
InputIt find_if(
    InputIt first, 
    InputIt last,
    UnaryPredicate f)
{
    for(; first != last; first++)
    {
        if(f(*first)) return first;
    }

    return last;
}
} // namespace ecs

#endif // _ECS_ALGORITHM_H_
