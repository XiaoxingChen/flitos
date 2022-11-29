#include <stdint.h>

uint32_t hookFunction(uint32_t func_id);
uint32_t getGlobalPointer(void);

void wrThreadYield()
{
    typedef void (*FuncNativeYield)();
    FuncNativeYield nativeYield = (FuncNativeYield)hookFunction(3);
    // load global pointer
    uint32_t cartridge_global_pointer;
    uint32_t firmware_global_pointer;
    // read `firmware_global_pointer` from system call
    firmware_global_pointer = getGlobalPointer();
    // read global pointer register and store to `cartridge_global_pointer`
    asm volatile ("mv %0, gp" : "=r"(cartridge_global_pointer));
    // write `firmware_global_pointer` and store to global pointer register
    asm volatile ("mv gp, %0" : : "r"(firmware_global_pointer));
    nativeYield();
    // write `cartridge_global_pointer` and store to global pointer register
    asm volatile ("mv gp, %0" : : "r"(cartridge_global_pointer));
    // unload global pointer
}
#if 0
void threadYield()
{
    wrThreadYield();
}
#endif

#if 1
void threadJoin(int tid)
{
    typedef void (*FuncKernelJoin)(int);
    FuncKernelJoin kjoin = (FuncKernelJoin)hookFunction(4);
    kjoin(tid);
}
#endif

void threadSleep(uint32_t ms)
{
    typedef void (*FuncKernelSleep)(uint32_t);
    FuncKernelSleep ksleep = (FuncKernelSleep)hookFunction(5);
    ksleep(ms);
}