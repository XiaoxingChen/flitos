#include <stdint.h>
#include "include/timer.h"

extern uint8_t _erodata[];
extern uint8_t _data[];
extern uint8_t _edata[];
extern uint8_t _sdata[];
extern uint8_t _esdata[];
extern uint8_t _bss[];
extern uint8_t _ebss[];

// Adapted from https://stackoverflow.com/questions/58947716/how-to-interact-with-risc-v-csrs-by-using-gcc-c-code
__attribute__((always_inline)) inline uint32_t

csr_mstatus_read(void) {
    uint32_t result;
    asm volatile ("csrr %0, mstatus" : "=r"(result));
    return result;
}

__attribute__((always_inline)) inline void csr_mstatus_write(uint32_t val) {
    asm volatile ("csrw mstatus, %0" : : "r"(val));
}

__attribute__((always_inline)) inline void csr_write_mie(uint32_t val) {
    asm volatile ("csrw mie, %0" : : "r"(val));
}

__attribute__((always_inline)) inline void csr_enable_interrupts(void) {
    asm volatile ("csrsi mstatus, 0x8");
}

__attribute__((always_inline)) inline void csr_disable_interrupts(void) {
    asm volatile ("csrci mstatus, 0x8");
}

#define INTRPT_PENDING  (*((volatile uint32_t *)0x40000004))
#define MTIME_LOW       (*((volatile uint32_t *)0x40000008))
#define MTIME_HIGH      (*((volatile uint32_t *)0x4000000C))
#define MTIMECMP_LOW    (*((volatile uint32_t *)0x40000010))
#define MTIMECMP_HIGH   (*((volatile uint32_t *)0x40000014))
#define CONTROLLER      (*((volatile uint32_t *)0x40000018))

void init(void) {
    uint8_t *Source = _erodata;
    uint8_t *Base = _data < _sdata ? _data : _sdata;
    uint8_t *End = _edata > _esdata ? _edata : _esdata;

    while (Base < End) {
        *Base++ = *Source++;
    }
    Base = _bss;
    End = _ebss;
    while (Base < End) {
        *Base++ = 0;
    }

    csr_write_mie(0x888);       // Enable all interrupt soruces
    csr_enable_interrupts();    // Global interrupt enable
    MTIMECMP_LOW = MTIME_LOW;
    MTIMECMP_HIGH = MTIME_HIGH;
}

extern volatile int global;
extern volatile uint32_t controller_status;
extern volatile char *VIDEO_MEMORY;
extern volatile int vip_seq;
extern volatile int cmd_seq;

void increase_timer() {
    uint64_t
    NewCompare = (((uint64_t)MTIMECMP_HIGH) << 32) | MTIMECMP_LOW;
    NewCompare += 100;
    MTIMECMP_LOW = NewCompare;
    MTIMECMP_HIGH = NewCompare >> 32;

}

void c_interrupt_handler(uint32_t mcause) {
    int flag = handle_time_interrupt(mcause);
    global++;
    controller_status = CONTROLLER;

    if(flag){
        increase_timer();
    }
    // increase_timer();

    if((INTRPT_PENDING & 0x2) > 0)
    {
        INTRPT_PENDING &= 0x2;
        vip_seq++;
    }
    if((INTRPT_PENDING & 0x4) > 0)
    {
        INTRPT_PENDING &= 0x4;
        cmd_seq++;
    }
}

volatile uint32_t aaa = 0;
uint32_t hookFunctionPointer(uint32_t fun_id);
uint32_t c_system_call(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t call){
    if(call == 0){
        return global;
    } else if (call == 1) {
        return CONTROLLER;
    }else if(call == 2){
        return vip_seq;
    } else if (call == 5) {
        aaa++;
        register_handler(a0);
        return 5;
    }
    // else if(call == 6){
    //     return writeTargetMem(a0, a1, a2);
    // }else if(call == 7){
    //     return writeTarget(a0, a1);
    // }
    else if(call == 8)
    {
        return hookFunctionPointer(a0);
    }
    else if(call == 9)
    {
        return cmd_seq;
    }
    return -1;
}

