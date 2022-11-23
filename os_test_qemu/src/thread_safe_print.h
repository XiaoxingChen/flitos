#if !defined(_THREAD_SAFE_PRINTF_H_)
#define _THREAD_SAFE_PRINTF_H_

#include "cs251_os.h"
#include "ecs_vector.h"
#include "ecs_string.h"
#include "uart_printf.h"


namespace cs251
{
    

template<typename T>
class ThreadSafeQueue
{
public:
    void enqueue(T&& val)
    {
        cs251::mutexFactoryInstance().lock(mtx_que_);
        if(size() == capacity())
        {
            storage_.resize(storage_.size() + 1);
        }
        storage_.at(idx_end_) = ecs::move(val);
        idx_end_ += 1;

        cs251::mutexFactoryInstance().unlock(mtx_que_);
        cs251::condFactoryInstance().notifyOne(cond_que_);
    }

    T dequeue()
    {
        cs251::mutexFactoryInstance().lock(mtx_que_);
        while(empty())
        {
            raw_printf("dequeue\n");
            cs251::condFactoryInstance().wait(cond_que_, mtx_que_);
        }
        T ret = storage_.at(idx_begin_);
        idx_begin_ = (idx_begin_ + 1) % storage_.size();
        cs251::mutexFactoryInstance().unlock(mtx_que_);
        return ret;
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

private:
    void invert_data(size_t p1, size_t p2)
    {
        T tmp;
        while(p1 < p2)
        {
            tmp = ecs::move(p1);
            p1 = ecs::move(p2);
            p2 = ecs::move(tmp);
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
    }

    bool idx_normalized() const
    {
        return idx_begin_ == 0;
    }
private:
    cs251::mutex_id_t mtx_que_;
    cs251::cond_id_t cond_que_;
    size_t idx_begin_;
    size_t idx_end_;
    ecs::vector<T> storage_;
};


void threadConsoleRunner(void* param);




extern void* p_console_queue;

inline ThreadSafeQueue<ecs::string>& consoleQueueInstance()
{
    if(p_console_queue == nullptr)
    {
        p_console_queue = new ThreadSafeQueue<ecs::string>();
    }
    return *static_cast<ThreadSafeQueue<ecs::string>*>(p_console_queue);
}

inline void initConsoleThread()
{
    // if(consoleQueueInstance().size())
    //     raw_printf("ddd test\n");
    cs251::schedulerInstance().create(threadConsoleRunner, nullptr);
    // cs251::schedulerInstance().create(threadConsoleRunner, &consoleQueueInstance());
}

#ifdef THREAD_SAFE_PRINT_IMPLEMENTATION

void* p_console_queue = nullptr;

void threadConsoleRunner(void* param)
{
    // ThreadSafeQueue<ecs::string>* p_que = static_cast<ThreadSafeQueue<ecs::string>*>(param);
    char * UART_MEMORY = (char*)(0x10000000);
    
    while(1)
    {
        // ecs::string msg;
        ecs::vector<int> msg(2);
        //msg = p_que->dequeue();
        // for(int i = 0; i < msg.size(); i++)
        // {
        //     UART_MEMORY[0] = msg.at(i);
        // }
    }
}

#endif
} // namespace cs251

#ifdef THREAD_SAFE_PRINT_IMPLEMENTATION

#else
int printf(const char *format, ...);

#endif

#endif // _THREAD_SAFE_PRINTF_H_
