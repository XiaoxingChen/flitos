#if !defined(_CS251_PIPE_H_)
#define _CS251_PIPE_H_

#include "cs251_thread_safe_queue.h"

namespace cs251
{

using pipe_id_t = int;

// https://opensource.com/article/19/4/interprocess-communication-linux-channels
class PipeInternal
{
public:
    PipeInternal() {  }
    void write(uint8_t* buff, size_t len)
    {
        q_.enqueue(buff, buff + len);
    }

    size_t read(uint8_t* buff, size_t len)
    {
        size_t i = 0;
        q_.dequeue([buff, &i](uint8_t byte){ buff[i] = byte; i++; }, len);
        return i;
    }

private:
    ThreadSafeQueue<uint8_t> q_;
};

class PipeFactory
{
public:
    pipe_id_t open() 
    {  
        seq_id_++;
        id_map_[seq_id_] = PipeInternal();
        return seq_id_;
    }
    void close(pipe_id_t mtx_id)
    {
        id_map_.erase(mtx_id);
    }
    size_t read(pipe_id_t id, uint8_t* buff, size_t len) 
    {
        return id_map_[id].read(buff, len); 
    }
    void write(pipe_id_t id, uint8_t* buff, size_t len) 
    { 
        id_map_[id].write(buff, len); 
    }
private:
    pipe_id_t seq_id_ = 0;
    ecs::map<pipe_id_t, PipeInternal> id_map_; 
};

extern PipeFactory* g_pipe_factory_;

inline PipeFactory& pipeFactoryInstance()
{
    if(g_pipe_factory_ == nullptr)
    {
        g_pipe_factory_ = new PipeFactory();
    }
    return *(g_pipe_factory_);
}

#ifdef CS251_OS_STATIC_OBJECTS_ON
PipeFactory* g_pipe_factory_ = nullptr;
#endif

} // namespace cs251

#endif // _CS251_PIPE_H_
