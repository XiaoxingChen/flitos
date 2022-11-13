#include <stdint.h>
#include "utils.h"
#include "cs251_os.h"

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
    csr_enable_interrupts();    // Global interrupt enable
    MTIMECMP_LOW = 1;
    MTIMECMP_HIGH = 0;
}
#ifdef __cplusplus
}
#endif
extern volatile int global;
extern volatile uint32_t controller_status;

inline uint64_t readMachineTime()
{
    uint64_t h1 = MTIME_HIGH;
    uint64_t l1 = MTIME_LOW;
    uint64_t h2 = MTIME_HIGH;
    uint64_t l2 = MTIME_LOW;
    if(h1 == h2) return ((h1 << 32) | l1);
    return (h2 << 32) | l2;
}

void increaseTimeCompare(uint32_t val)
{
    uint64_t NewCompare = readMachineTime();
    NewCompare += val;
    MTIMECMP_HIGH = NewCompare>>32;
    MTIMECMP_LOW = NewCompare;
}

#ifdef __cplusplus
extern "C"{
#endif
void c_interrupt_handler(void){
#if 0
    uint64_t NewCompare = (((uint64_t)MTIMECMP_HIGH)<<32) | MTIMECMP_LOW;
    NewCompare += 5000;
    MTIMECMP_HIGH = NewCompare>>32;
    MTIMECMP_LOW = NewCompare;
    global++;
    controller_status = CONTROLLER;

    uint64_t curr_timer = readMachineTime();
    if(NewCompare > curr_timer)
        cs251::thread_yield();
#else
    uint64_t NewCompare = readMachineTime() + 5000;
    MTIMECMP_HIGH = NewCompare>>32;
    MTIMECMP_LOW = NewCompare;
    global++;
    controller_status = CONTROLLER;
    cs251::thread_yield();
#endif
}
#ifdef __cplusplus
}
#endif

