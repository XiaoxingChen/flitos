#include <stdint.h>
#include <string.h>

namespace cs251{
    int thread_yield();
}


uint32_t* regAddrFromMemHandle(uint32_t mem_handle)
{
    return (uint32_t*)(mem_handle + 0x50000000);
}

uint32_t memLengthCheck(uint32_t mem_handle, uint32_t mem_len)
{
    return (uint32_t)0;
}

uint32_t writeTargetMem(uint32_t mem_handle, uint32_t source_addr, uint32_t mem_len)
{
    uint32_t* addr = regAddrFromMemHandle(mem_handle);
    uint32_t result = memLengthCheck(mem_handle, mem_len);
    if(result != 0) return result;
    // memcpy(addr, (void*)source_addr, mem_len);
    for(uint32_t i = 0; i < mem_len; i++) ((uint8_t*)addr)[i] = ((uint8_t*)source_addr)[i];
    return 0;
}

uint32_t writeTarget(uint32_t mem_handle, uint32_t value)
{
    uint32_t* addr = regAddrFromMemHandle(mem_handle);
    uint32_t result = memLengthCheck(mem_handle, 4);
    if(result != 0) return result;
    *addr = value;
    return 0;
}

uint32_t hookFunctionPointer(uint32_t fun_id)
{
    if(1 == fun_id)
    {
        return (uint32_t)&writeTargetMem;
    }else if(2 == fun_id)
    {
        return (uint32_t)&writeTarget;
    }else if(3 == fun_id)
    {
        return (uint32_t)&cs251::thread_yield;
    }
    return 0;
}