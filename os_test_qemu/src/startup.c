#include <stdint.h>
#include "utils.h"
#include "cs251_os.h"
#include "uart_printf.h"

extern uint8_t _erodata[];
extern uint8_t _data[];
extern uint8_t _edata[];
extern uint8_t _sdata[];
extern uint8_t _esdata[];
extern uint8_t _bss[];
extern uint8_t _ebss[];


#define MTIME_LOW       (*((volatile uint32_t *)0x0200bff8))
#define MTIME_HIGH      (*((volatile uint32_t *)0x0200bffC))
#define MTIMECMP_LOW    (*((volatile uint32_t *)0x02004000))
#define MTIMECMP_HIGH   (*((volatile uint32_t *)0x02004004))
// #define CONTROLLER      (*((volatile uint32_t *)0x40000018))
uint32_t CONTROLLER;

#if 0
static void callConstructors()
{
    // Start and end points of the constructor list,
    // defined by the linker script.
    extern void (*__init_array_start)();
    extern void (*__init_array_end)();

    // Call each function in the list.
    // We have to take the address of the symbols, as __init_array_start *is*
    // the first function pointer, not the address of it.
    for (void (**p)() = &__init_array_start; p < &__init_array_end; ++p) {
        (*p)();
    }
}
#endif

#ifdef __cplusplus
extern "C"{
#endif
void init(void){
    uint8_t *Source = _erodata;
    uint8_t *Base = _data < _sdata ? _data : _sdata;
    uint8_t *End = _edata > _esdata ? _edata : _esdata;

    while(Base < End){
        *Base++ = *Source++;
    }
    Base = _bss;
    End = _ebss;
    while(Base < End){
        *Base++ = 0;
    }
    // callConstructors();

    csr_write_mie(0x888);       // Enable all interrupt soruces
    // csr_enable_interrupts();    // Global interrupt enable
    MTIMECMP_LOW = 1;
    MTIMECMP_HIGH = 0;
}
#ifdef __cplusplus
}
#endif
extern volatile int global;
extern volatile uint32_t controller_status;

#ifdef __cplusplus
extern "C"{
#endif

void illegalTrap()
{
    uint32_t mcause_code;
    asm volatile ("csrr %0, mcause" : "=r"(mcause_code));
    
    if(mcause_code < 20)
    {
        printf("===== Segment Fault! =====\n");
        printContextSnapshot();
        
        while(1);
    }
}

int context_switch_cnt = 0;

void c_interrupt_handler(void){
    *nestCriticalCount() += 1;
    illegalTrap();
    // uint32_t timecmp_step = 5000;
    // uint64_t curr_timer = readMachineTime();

    // uint64_t NewCompare = (((uint64_t)MTIMECMP_HIGH)<<32) | MTIMECMP_LOW;
    // while (NewCompare <= curr_timer)
    //     NewCompare += timecmp_step;
    // MTIMECMP_HIGH = NewCompare>>32;
    // MTIMECMP_LOW = NewCompare;
    increaseTimeCompare(5000);
    global++;
    controller_status = CONTROLLER;
#if 1
    // if(curr_timer < NewCompare && NewCompare < curr_timer + timecmp_step)
    cs251::schedulerInstance().inInterruptYield();
#else

    char * UART_MEMORY = (char*)(0x10000000);

    if((UART_MEMORY[5] & 1) && UART_MEMORY[0] == 'a')
    {
        printf("Do context switch, seq: %d\n", context_switch_cnt++);
        cs251::schedulerInstance().inInterruptYield();
    }
#endif
    *nestCriticalCount() -= 1;
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"{
#endif
uint32_t c_system_call(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t call){
    *nestCriticalCount() += 1;

if(call == 13)
    {
        return (uint32_t)cs251::mutexFactoryInstance().create();
    }else if(call == 14)
    {
        cs251::mutexFactoryInstance().destroy(a0);
    }
    else if(call == 15)
    {
        // not tested
        cs251::mutexFactoryInstance().lock(a0);
    }else if(call == 16)
    {
        // not tested
        cs251::mutexFactoryInstance().unlock(a0);
    }else if(call == 17)
    {
        cs251::schedulerInstance().join(a0);
    }

    *nestCriticalCount() -= 1;
    return -1;
}
#ifdef __cplusplus
}
#endif


