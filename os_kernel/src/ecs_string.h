#if !defined(_ECS_STRING_H_)
#define _ECS_STRING_H_

#include "ecs_vector.h"
namespace ecs
{
template<class Allocator=allocator<char>>
class basic_string: public vector<char, Allocator>
{
public:
    using BaseType = vector<char, Allocator>;
    char* c_str() { return BaseType::data(); }
    size_t size() const { return BaseType::size() - 1; }

    basic_string (): BaseType(1) { BaseType::at(0) = '\x00'; };


    basic_string (const basic_string& rhs): BaseType(rhs)
    {
    }

    basic_string (basic_string&& rhs): BaseType(ecs::move(rhs))
    {
        rhs.push_back('\x00');
    }

    basic_string (const char* c_str)
    {
        size_t idx = 0;
        while(1)
        {
            BaseType::push_back(c_str[idx]);
            if(c_str[idx] == 0) break;
            idx++;
        }
    }
    void operator += (const basic_string& rhs)
    {
        size_t old_size = BaseType::size();
        BaseType::resize(old_size + rhs.size() + 1);
        for(size_t i = 0; i < rhs.size(); i++)
        {
            BaseType::at(i + old_size) = rhs.at(i);
        }
        BaseType::at(size()) = '\x00';
    }

    basic_string operator + (const basic_string& rhs) const
    {
        basic_string ret;
        ret += *this;
        ret += rhs;
        return ret;
    }

    void operator = (basic_string&& rhs)
    {
        BaseType::swap(rhs);
    }

    void operator = (const basic_string& rhs)
    {
        for(size_t i = 0; i < static_cast<BaseType>(rhs).size(); i++)
        {
            BaseType::at(i) = rhs.at(i);
        }
    }
private:
};

using string = basic_string<>;
} // namespace ecs



#endif // _ECS_STRING_H_
