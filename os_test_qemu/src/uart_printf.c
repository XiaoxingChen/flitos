#include "nanoprintf.h"
#include "stdint.h"
#include "port_riscv.h"
#include "utils.h"
#include "uart_printf.h"
#include "thread_safe_print.h"

int raw_printf(const char *format, ...)
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

int printf(const char *format, ...)
{
    unsigned int buff_len = 1024;
    char vsnprintf_buff[1024];
    va_list args;
    int n = buff_len;
    va_start(args, format);
	n = npf_vsnprintf(vsnprintf_buff, buff_len, format, args);
	va_end(args);
    
#if 0
    ecs::string tmp_str(vsnprintf_buff);
    cs251::consoleQueueInstance().enqueue(ecs::move(tmp_str));
#else
    char * UART_MEMORY = (char*)(0x10000000);
    for(int i = 0; i < n; i++)
    {
        UART_MEMORY[0] = vsnprintf_buff[i];
    }
#endif
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
    raw_printf("Context Snapshot\n");
    raw_printf("ra: 0x%X\n", addr[offset_ra]);
    raw_printf("sp: 0x%X\n", addr[offset_sp]);
    raw_printf("gp: 0x%X\n", addr[offset_gp]);
    raw_printf("tp: 0x%X\n", addr[offset_tp]);
    raw_printf("t0: 0x%X\n", addr[offset_t0]);
    raw_printf("t1: 0x%X\n", addr[offset_t1]);
    raw_printf("t2: 0x%X\n", addr[offset_t2]);
    raw_printf("s0: 0x%X\n", addr[offset_s0]);
    raw_printf("s1: 0x%X\n", addr[offset_s1]);
    raw_printf("a0: 0x%X\n", addr[offset_a0]);
    raw_printf("a1: 0x%X\n", addr[offset_a1]);
    raw_printf("a2: 0x%X\n", addr[offset_a2]);
    raw_printf("a3: 0x%X\n", addr[offset_a3]);
    raw_printf("a4: 0x%X\n", addr[offset_a4]);
    raw_printf("a5: 0x%X\n", addr[offset_a5]);
    
    raw_printf("mepc: 0x%X\n", addr[offset_mepc]);
    raw_printf("mcause: 0x%X. %s.\n", addr[offset_mcause], addr[offset_mcause] < 8 ? mcause_table[addr[offset_mcause]] : "no description");
    raw_printf("mstatus: 0x%X\n", addr[offset_mstatus]);
}

#if defined(__cplusplus)
}
#endif // __cplusplus