/**
******************************************************************************
* @file    ecs_vector.h
* @author  Chenxx
* @version V1.0
* @date    2022-11-05
* @brief   simple vector implementation.
******************************************************************************/
#if !defined(__ECS_VECTOR_H__)
#define __ECS_VECTOR_H__

#include <stdlib.h>
#include <algorithm>
#include <cstring>
#include "ecs_traits.h"
namespace ecs
{

inline size_t findMemSize(size_t val)
{
    size_t ret = 1;
    while(ret < val) ret <<= 1;
    return ret;
}

template<typename T>
class vector
{
private:
    size_t size_ = 0;
    size_t cap_ = 0;
    T* mem_ = nullptr;
    void reallocateMemory(size_t desire)
    {
        T* old_mem = mem_;
        cap_ = std::max<size_t>(2, findMemSize(desire));
        mem_ = static_cast<T*>(malloc(cap_ * sizeof(T)));
        if(old_mem != nullptr)
        {
            memcpy(mem_, old_mem, size_ * sizeof(T));
            free(old_mem);
        }
    }
public:
    vector(){};
    vector(size_t n) { resize(n); };

    ~vector()
    {
        for(size_t i = 0; i < size(); i++)
        {
            mem_[i].~T();
        }
        free(mem_);
    }

    void swap(vector& rhs)
    {
        size_t tmp;
        tmp = rhs.size_;
        rhs.size_ = size_;
        size_ = tmp;

        tmp = rhs.cap_;
        rhs.cap_ = cap_;
        cap_ = tmp;

        T* tmp_mem;
        tmp_mem = rhs.mem_;
        rhs.mem_ = mem_;
        mem_ = tmp_mem;
    }

    vector(vector&& rhs)
    {
        swap(rhs);
    }

    void operator = (vector&& rhs)
    {
        swap(rhs);
    }

    vector(const vector& rhs)
    {
        resize(rhs.size());
        for(size_t i = 0; i < rhs.size(); i++) at(i) = rhs.at(i);
    }

    void reserve(size_t n) { reallocateMemory(n); }
    size_t capacity() const { return cap_; }
    size_t size() const { return size_; }
    void resize(size_t n) {
        if(n > cap_) reallocateMemory(n);
        size_t old_size = size_;
        size_ = n;
        for(size_t i = old_size; i < n; i++)
        {
            new (&mem_[i]) T;
        }
        for(size_t i = size_; i < old_size; i++)
        {
            mem_[i].~T();
        }

    }
    void push_back(const T& v)
    {
        if(size_ + 1 > cap_)
        {
            reallocateMemory( cap_ * 2 );
        }
        mem_[size_] = v;
        size_ += 1;
    }
    void pop_back()  {  size_ = 0 == size_ ? 0 : size_ - 1; }

    const T& operator [] (size_t i) const { return mem_[i]; }

    T& operator [] (size_t i) { return mem_[i]; }

    const T& at(size_t i) const  { return mem_[i]; }
    T& at(size_t i)  { return mem_[i]; }

    const T& back() const  { return mem_[size_ - 1]; }
    T& back()  { return mem_[size_ - 1]; }

    const T* data() const { return mem_; }
    T* data() { return mem_; }

    bool empty() const { return 0 == size_; }
    void clear() 
    {
        resize(0);
    }
};


} // namespace ecs


#endif // __ECS_VECTOR_H__
