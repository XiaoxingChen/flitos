#include <stdint.h>
#include "include/timer.h"
#define ALLOCATOR_IMPLEMENTATION
#include "cs251_os.h"
#include "ecs_allocator.h"
#include "utils.h"

extern uint8_t _erodata[];
extern uint8_t _data[];
extern uint8_t _edata[];
extern uint8_t _sdata[];
extern uint8_t _esdata[];
extern uint8_t _bss[];
extern uint8_t _ebss[];
extern uint8_t __global_pointer$[];
#if 0
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
#endif

#define INTRPT_PENDING  (*((volatile uint32_t *)0x40000004))
#define MTIME_LOW       (*((volatile uint32_t *)0x40000008))
#define MTIME_HIGH      (*((volatile uint32_t *)0x4000000C))
#define MTIMECMP_LOW    (*((volatile uint32_t *)0x40000010))
#define MTIMECMP_HIGH   (*((volatile uint32_t *)0x40000014))
#define CONTROLLER      (*((volatile uint32_t *)0x40000018))

#ifdef __cplusplus
extern "C"{
#endif

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
    MTIMECMP_LOW = 1;
    MTIMECMP_HIGH = 0;
}

#ifdef __cplusplus
}
#endif

extern volatile int global;
extern volatile uint32_t controller_status;
extern volatile char *VIDEO_MEMORY;
extern volatile int vip_seq;
extern volatile int cmd_seq;

#ifdef __cplusplus
extern "C"{
#endif

void c_interrupt_handler(uint32_t mcause) {
    increaseNestCriticalCount();

    int flag = handle_time_interrupt(mcause);
    
    controller_status = CONTROLLER;

    if(flag){
        global++;
        increaseTimeCompare(100);
        cs251::sleepTimerInstance().updateTick();
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
    cs251::schedulerInstance().inInterruptYield();

    decreaseNestCriticalCount();
}

#ifdef __cplusplus
}
#endif

volatile uint32_t aaa = 0;
uint32_t hookFunctionPointer(uint32_t fun_id);
uint32_t writeTargetMem(uint32_t mem_handle, uint32_t source_addr, uint32_t mem_len);
uint32_t writeTarget(uint32_t mem_handle, uint32_t value);
#ifdef __cplusplus
extern "C"{
#endif
uint32_t c_system_call(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t call){
    increaseNestCriticalCount();
    uint32_t ret = 0xffffffff;

    if(call == 0){
        ret = global;
    } else if (call == 1) {
        ret = CONTROLLER;
    }else if(call == 2){
        ret = vip_seq;
    } else if (call == 5) {
        aaa++;
        register_handler(a0);
        ret = 5;
    }
    else if(call == 6){
        ret = writeTargetMem(a0, a1, a2);
    }else if(call == 7){
        ret = writeTarget(a0, a1);
    }
    else if(call == 8)
    {
        ret = hookFunctionPointer(a0);
    }
    else if(call == 9)
    {
        ret = cmd_seq;
    }
    else if(call == 10)
    {
        typedef void (*f_thread)(void*);
        uint32_t thread_id = cs251::schedulerInstance().create((f_thread)a0, (void*)a1, a2);
        ret = thread_id;
    }
    else if(call == 11)
    {
        cs251::schedulerInstance().inInterruptYield();
    }else if(call == 12)
    {
        asm volatile ("mv %0, gp" : "=r"(ret));
    }else if(call == 13)
    {
        ret = (uint32_t)cs251::mutexFactoryInstance().create();
    }else if(call == 14)
    {
        cs251::mutexFactoryInstance().destroy(a0);
    }
    else if(call == 15)
    {
        // by passed
        cs251::mutexFactoryInstance().lock(a0);
    }else if(call == 16)
    {
        // by passed
        cs251::mutexFactoryInstance().unlock(a0);
    }else if(call == 17)
    {
        cs251::schedulerInstance().join(a0);
    }else if (call == 18)
    {
        cs251::sleepTimerInstance().sleep(a0);
    }else if(call == 19)
    {
        // test required
        ret = cs251::condFactoryInstance().create();
    }else if(call == 20)
    {
        // test required
        cs251::condFactoryInstance().destroy(a0);
    }else if(call == 21)
    {
        // test required
        cs251::condFactoryInstance().notifyOne(a0);
    }else if(call == 22)
    {
        // test required
        cs251::condFactoryInstance().notifyAll(a0);
    }else if(call == 23)
    {
        // test required
        cs251::condFactoryInstance().wait(a0, a1);
    }else if(call == 24)
    {
        // test required
        ret = cs251::pipeFactoryInstance().open();
    }else if(call == 25)
    {
        // test required
        cs251::pipeFactoryInstance().close(a0);
    }else if(call == 26)
    {
        // test required
        ret = cs251::pipeFactoryInstance().read(a0, (uint8_t*)a1, a2);
    }else if(call == 27)
    {
        // test required
        cs251::pipeFactoryInstance().write(a0, (uint8_t*)a1, a2);
    }else if(call == 28)
    {
        // test required
        cs251::mutex_id_t mtx = ecs::allocatorMutexInstance();
        cs251::mutexFactoryInstance().lock(mtx);
        ret = (uint32_t)malloc(a0);
        cs251::mutexFactoryInstance().unlock(mtx);
    }else if(call == 29)
    {
        // test required
        cs251::mutex_id_t mtx = ecs::allocatorMutexInstance();
        cs251::mutexFactoryInstance().lock(mtx);
        free((void*)a0);
        cs251::mutexFactoryInstance().unlock(mtx);
    }

    decreaseNestCriticalCount();
    return ret;
}
#ifdef __cplusplus
}
#endif
