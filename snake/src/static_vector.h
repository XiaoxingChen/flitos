#if !defined(_STATIC_VEC_H_)
#define _STATIC_VEC_H_

#include <array>

template<typename T, size_t N>
class StaticVector
{
private:
    size_t len_ = 0;
    std::array<T, N> mem_;
public:
    StaticVector(){};
    size_t capacity() const { return N; }
    size_t size() const { return len_; }
    void push_back(const T& v)
    {
        if(len_ + 1 == N) return;
        mem_[len_] = v;
        len_ += 1;
    }
    void pop_back()  {  len_ = 0 == len_ ? 0 : len_ - 1; }

    const T& at(size_t i) const  { return mem_[i]; }
    T& at(size_t i)  { return mem_[i]; }

    const T& back() const  { return mem_[len_ - 1]; }
    T& back()  { return mem_[len_ - 1]; }

    const T* data() const { return &mem_[0]; }
    T* data() { return &mem_[0]; }

    bool empty() const { return 0 == len_; }
    void clear() { len_ = 0; }
};



#endif // _STATIC_VEC_H_
