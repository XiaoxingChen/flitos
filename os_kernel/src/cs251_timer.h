#if !defined(_CS251_TIMER_H_)
#define _CS251_TIMER_H_

#include <stddef.h>
#include "cs251_mutex.h"
#include "cs251_condition_variable.h"

namespace cs251
{

class SleepTimer
{
public:
    SleepTimer()
    {
        mtx_for_sleep_ = mutexFactoryInstance().create();
        cond_for_sleep_ = condFactoryInstance().create();
    }
    void sleep(size_t cnt)
    {
        size_t wake_tick = systick_ + cnt;
        mutexFactoryInstance().lock(mtx_for_sleep_);
        
        while(systick_ < wake_tick)
        {
            condFactoryInstance().wait(cond_for_sleep_, mtx_for_sleep_);
        }
        mutexFactoryInstance().unlock(mtx_for_sleep_);
    }

    void updateTick()
    { 
        systick_++; 
        condFactoryInstance().notifyAll(cond_for_sleep_);
    }

private:
    volatile size_t systick_ = 0;
    mutex_id_t mtx_for_sleep_ = -1;
    cond_id_t cond_for_sleep_ = -1;
};

extern void* g_mutex_factory;
extern void* g_condition_variable_factory;
extern SleepTimer* g_sleep_timer;

inline SleepTimer& sleepTimerInstance()
{
    if(g_sleep_timer == nullptr)
        g_sleep_timer = new SleepTimer();
    return *g_sleep_timer;
}

} // namespace cs251


#endif // _CS251_TIMER_H_
