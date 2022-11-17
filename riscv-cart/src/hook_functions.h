#if !defined(_HOOK_FUNCTION_H_)
#define _HOOK_FUNCTION_H_

typedef void (*FuncNativeYield)();
// typedef uint32_t (*FuncWriteTargetMem)(uint32_t mem_handle, uint32_t source_addr, uint32_t mem_len);
// typedef uint32_t (*FuncWriteTarget)(uint32_t mem_handle, uint32_t value);

// extern FuncWriteTargetMem writeTargetMem;
// extern FuncWriteTarget writeTarget;
extern FuncNativeYield nativeYield;

void initHookFunctions();
void wrThreadYield();

#ifdef HOOK_FUNCTIONS_STATIC_OBJECTS_ON
// FuncWriteTargetMem writeTargetMem;
// FuncWriteTarget writeTarget;
FuncNativeYield nativeYield;

uint32_t hookFunction(uint32_t func_id);
uint32_t getGlobalPointer(void);

void initHookFunctions()
{
    // writeTargetMem = (FuncWriteTargetMem)hookFunction(1);
    // writeTarget = (FuncWriteTarget)hookFunction(2);
    nativeYield = (FuncNativeYield)hookFunction(3);
}

void wrThreadYield()
{
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
#endif





#endif // _HOOK_FUNCTION_H_
