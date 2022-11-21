#include "utils.h"
#include "nanoprintf.h"

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

int32_t nest_critical_cnt;

int32_t* nestCriticalCount()
{
    return &nest_critical_cnt;
}

void enable_interrupts()
{
    (*nestCriticalCount())--;
    if(*nestCriticalCount() <= 0)
    {
        *nestCriticalCount() = 0;
        csr_enable_interrupts();
    }
}

void disable_interrupts()
{
    if(*nestCriticalCount() <= 0)
    {
        *nestCriticalCount() = 0;
        csr_disable_interrupts();
    }
    (*nestCriticalCount())++;
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


#if defined(__cplusplus)
}
#endif // __cplusplus