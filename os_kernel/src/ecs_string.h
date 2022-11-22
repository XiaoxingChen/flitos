#if !defined(_ECS_STRING_H_)
#define _ECS_STRING_H_

#include "ecs_vector.h"

namespace ecs
{
    
class string: public vector<char>
{
public:
    using BaseType = vector<char>;
    char* c_str() { return BaseType::data(); }
    size_t size() const { return BaseType::size() - 1; }

    string (): BaseType(1) { at(0) = '\x00'; };


    string (const string& rhs): vector(rhs)
    {
    }

    string (string&& rhs): vector(ecs::move(rhs))
    {
        rhs.push_back('\x00');
    }

    string (const char* c_str)
    {
        size_t idx = 0;
        while(1)
        {
            push_back(c_str[idx]);
            if(c_str[idx] == 0) break;
            idx++;
        }
    }
    void operator += (const string& rhs)
    {
        size_t old_size = size();
        resize(old_size + rhs.size() + 1);
        for(size_t i = 0; i < rhs.size(); i++)
        {
            at(i + old_size) = rhs.at(i);
        }
        at(size()) = '\x00';
    }

    string operator + (const string& rhs) const
    {
        string ret;
        ret += *this;
        ret += rhs;
        return ret;
    }

    void operator = (string&& rhs)
    {
        swap(rhs);
    }

    void operator = (const string& rhs)
    {
        for(size_t i = 0; i < static_cast<BaseType>(rhs).size(); i++)
        {
            at(i) = rhs.at(i);
        }
    }
private:
};
} // namespace ecs



#endif // _ECS_STRING_H_
