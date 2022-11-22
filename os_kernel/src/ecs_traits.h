#if !defined(_ECS_TRAITS_H_)
#define _ECS_TRAITS_H_

namespace ecs
{
template< class T > struct remove_reference      { typedef T type; };
template< class T > struct remove_reference<T&>  { typedef T type; };
template< class T > struct remove_reference<T&&> { typedef T type; };

template< class T >
typename remove_reference<T>::type&& move( T&& t )
{
    return static_cast<typename remove_reference<T>::type&&>(t);
}
} // namespace ecs

#endif // _ECS_TRAITS_H_
