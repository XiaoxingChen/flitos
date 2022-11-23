#if !defined(_ECS_QUEUE_H_)
#define _ECS_QUEUE_H_

#include "ecs_vector.h"

namespace ecs
{
template <typename T> class allocator;
template<typename T, class Allocator=allocator<T>>
class deque
{
public:
    using ThisType = deque<T, Allocator>;
    deque() = default;

    void push_back(const T& val)
    {
        if(size() == capacity())
        {
            normalize_idx();
            storage_.resize(capacity() + 2);
        }
        storage_.at(idx_end_) = val;
        idx_end_ = (idx_end_ + 1) % storage_.size();
    }

    void pop_front()
    {
        idx_begin_ = (idx_begin_ + 1) % storage_.size();
    }

    const T& front()
    {
        return storage_.at(idx_begin_);
    }

    size_t capacity() const
    {
        if(storage_.empty()) return 0;
        return storage_.size() - 1;
    }

    size_t size() const
    {
        if(idx_end_ >= idx_begin_) return idx_end_ - idx_begin_;
        return idx_end_ + storage_.size() - idx_begin_;
    }
    bool empty() const
    {
        return 0 == size();
    }

    T& at(size_t idx)
    {
        size_t raw_idx = (idx_begin_ + idx) % storage_.size();
        return storage_.at(raw_idx);
    }

    const T& at(size_t idx) const
    {
        return const_cast<ThisType*>(this)->at(idx);
    }


private:
    void invert_data(size_t p1, size_t p2)
    {
        T tmp;
        while(p1 < p2)
        {
            tmp = ecs::move(storage_.at(p1));
            storage_.at(p1) = ecs::move(storage_.at(p2));
            storage_.at(p2) = ecs::move(tmp);
            p1++;
            p2--;
        }
    }
    void normalize_idx()
    {
        if(idx_normalized()) return;

        invert_data(0, idx_begin_ - 1);
        invert_data(idx_begin_, storage_.size() - 1);
        invert_data(0, storage_.size() - 1);
        idx_begin_ = 0;
        idx_end_ = storage_.size() - 1;
        
    }

    bool idx_normalized() const
    {
        return idx_begin_ == 0;
    }

#if 0
    void debug()
    {
        std::cout << "begin, end: " << idx_begin_ << ", " << idx_end_ << "; store size: " << storage_.size() << ";";
        for(size_t i = 0; i < storage_.size(); i++)
        {
            std::cout << storage_.at(i) << " ";
        }std::cout << std::endl;

    }
#endif

private:
    size_t idx_begin_ = 0;
    size_t idx_end_ = 0;
    ecs::vector<T, Allocator> storage_;
};


template <typename T, class AllocatorQ=ecs::allocator<T>>
using queue = deque<T, AllocatorQ>;

} // namespace ecs

#endif // _ECS_QUEUE_H_
