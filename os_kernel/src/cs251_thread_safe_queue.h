#if !defined(_CS251_THREAD_SAFE_QUEUE_H_)
#define _CS251_THREAD_SAFE_QUEUE_H_

#include "ecs_allocator.h"
#include "ecs_queue.h"

namespace cs251
{

template<typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue()
    {
        mtx_que_ = cs251::mutexFactoryInstance().create();
        cond_que_ = cs251::condFactoryInstance().create();
    }
    void enqueue(const T& val)
    {
        cs251::mutexFactoryInstance().lock(mtx_que_);
        q_.push_back(val);

        cs251::mutexFactoryInstance().unlock(mtx_que_);
        cs251::condFactoryInstance().notifyOne(cond_que_);
    }

    template<typename Iterator>
    void enqueue(Iterator begin, Iterator end)
    {
        cs251::mutexFactoryInstance().lock(mtx_que_);

        for(auto it = begin; it != end; it++)
        {
            q_.push_back(*it);
        }

        cs251::mutexFactoryInstance().unlock(mtx_que_);
        cs251::condFactoryInstance().notifyOne(cond_que_);
    }

    T dequeue()
    {
        cs251::mutexFactoryInstance().lock(mtx_que_);
        while(empty())
        {
            cs251::condFactoryInstance().wait(cond_que_, mtx_que_);
        }

        T ret = q_.front();
        q_.pop_front();

        cs251::mutexFactoryInstance().unlock(mtx_que_);
        return ret;
    }

    template<typename Func>
    void dequeue(Func collect_func, size_t n)
    {
        cs251::mutexFactoryInstance().lock(mtx_que_);
        while(empty())
        {
            // raw_printf("dequeue\n");
            cs251::condFactoryInstance().wait(cond_que_, mtx_que_);
        }

        size_t pop_n = n < size() ? n : size();
        for(size_t i = 0; i < pop_n; i++)
        {
            collect_func(q_.front());
            q_.pop_front();
        }
        cs251::mutexFactoryInstance().unlock(mtx_que_);
    }

    size_t size() const
    {
        return q_.size();
    }
    bool empty() const
    {
        return q_.empty();
    }

private:
    cs251::mutex_id_t mtx_que_;
    cs251::cond_id_t cond_que_;
    ecs::deque<T> q_;
};

} // namespace cs251

#endif // _CS251_THREAD_SAFE_QUEUE_H_
