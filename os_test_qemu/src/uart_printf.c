#include "nanoprintf.h"
#include "stdint.h"

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

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


uint32_t context_shot[20];
void printContextSnapshot()
{
    uint32_t * addr = context_shot;
    printf("Context Snapshot\n");
    printf("ra: 0x%X\n", addr[0]);
    printf("sp: 0x%X\n", addr[1]);
    printf("gp: 0x%X\n", addr[2]);
    printf("tp: 0x%X\n", addr[3]);
    printf("t0: 0x%X\n", addr[4]);
    printf("t1: 0x%X\n", addr[5]);
    printf("t2: 0x%X\n", addr[6]);
    printf("s0: 0x%X\n", addr[7]);
    printf("s1: 0x%X\n", addr[8]);
    printf("a0: 0x%X\n", addr[9]);
    printf("a1: 0x%X\n", addr[10]);
    printf("a2: 0x%X\n", addr[11]);
    printf("a3: 0x%X\n", addr[12]);
    printf("a4: 0x%X\n", addr[13]);
    printf("a5: 0x%X\n", addr[14]);
    
    printf("mepc: 0x%X\n", addr[15]);
    printf("mcause: 0x%X\n", addr[16]);
}

#if defined(__cplusplus)
}
#endif // __cplusplus