#include "utils.h"
#include "nanoprintf.h"
#if defined(VIRT_CLINT)
#include "uart_printf.h"
#endif
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

int32_t nest_critical_cnt;

int32_t* nestCriticalCount()
{
    return &nest_critical_cnt;
}

void increaseNestCriticalCount() 
{ 
    nest_critical_cnt++; 
    // raw_printf("nest_critical_cnt ++ to: %d\n", nest_critical_cnt);
}

void decreaseNestCriticalCount() 
{ 
    nest_critical_cnt--; 
    // raw_printf("nest_critical_cnt -- to: %d\n", nest_critical_cnt);
}

int isCriticalBound()
{
    if(nest_critical_cnt < 0) nest_critical_cnt = 0;
    return nest_critical_cnt == 0;
}

void enable_interrupts()
{
    decreaseNestCriticalCount();
    if(isCriticalBound())
    {
        csr_enable_interrupts();
    }
}

void disable_interrupts()
{
    if(isCriticalBound())
    {
        csr_disable_interrupts();
    }
    increaseNestCriticalCount();
}

int line_printf(int idx, const char *format, ...)
{
    char *VIDEO_MEMORY = (char *)(0x50000000 + 0xFE800);
    unsigned int buff_len = 0x40;
    va_list args;
    int n = buff_len;
    va_start(args, format);
	n = npf_vsnprintf(&VIDEO_MEMORY[0x40 * idx], buff_len, format, args);
	va_end(args);

    return n;
}

void global_ptr_write(uint32_t val)
{
    asm volatile ("mv gp, %0" : : "r"(val));
}

uint32_t global_ptr_read(void)
{
    uint32_t result;
    asm volatile ("mv %0, gp" : "=r"(result));
    return result;
}


#if defined(__cplusplus)
}
#endif // __cplusplus