#include "nanoprintf.h"
#include "stdint.h"
#include "port_riscv.h"
#include "utils.h"

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


uint32_t context_shot[context_size];
const char* mcause_table[8] = {
    "Instruction address misaligned", 
    "Instruction access fault", 
    "Illegal instruction",
    "",
    "",
    "Load access fault",
    "",
    "Store/AMO access fault"};
void printContextSnapshot()
{
    uint32_t * addr = context_shot;
    printf("Context Snapshot\n");
    printf("ra: 0x%X\n", addr[offset_ra]);
    printf("sp: 0x%X\n", addr[offset_sp]);
    printf("gp: 0x%X\n", addr[offset_gp]);
    printf("tp: 0x%X\n", addr[offset_tp]);
    printf("t0: 0x%X\n", addr[offset_t0]);
    printf("t1: 0x%X\n", addr[offset_t1]);
    printf("t2: 0x%X\n", addr[offset_t2]);
    printf("s0: 0x%X\n", addr[offset_s0]);
    printf("s1: 0x%X\n", addr[offset_s1]);
    printf("a0: 0x%X\n", addr[offset_a0]);
    printf("a1: 0x%X\n", addr[offset_a1]);
    printf("a2: 0x%X\n", addr[offset_a2]);
    printf("a3: 0x%X\n", addr[offset_a3]);
    printf("a4: 0x%X\n", addr[offset_a4]);
    printf("a5: 0x%X\n", addr[offset_a5]);
    
    printf("mepc: 0x%X\n", addr[offset_mepc]);
    printf("mcause: 0x%X. %s.\n", addr[offset_mcause], addr[offset_mcause] < 8 ? mcause_table[addr[offset_mcause]] : "no description");
    printf("mstatus: 0x%X\n", addr[offset_mstatus]);
}

#if defined(__cplusplus)
}
#endif // __cplusplus