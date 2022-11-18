#include "nanoprintf.h"

int printf(const char *format, ...)
{
    char * UART_MEMORY = (char*)(0x10000000);
    unsigned int buff_len = 1024;
    char vsnprintf_buff[1024];
    va_list args;
    int n = buff_len;
    va_start(args, format);
	n = npf_vsnprintf(vsnprintf_buff, buff_len, format, args);
	va_end(args);

    for(int i = 0; i < n; i++)
    {
        UART_MEMORY[0] = vsnprintf_buff[i];
    }
    return n;
}