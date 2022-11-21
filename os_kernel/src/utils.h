#if !defined(_UTILS_H_)
#define _UTILS_H_
#include <stdint.h>
#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// Adapted from https://stackoverflow.com/questions/58947716/how-to-interact-with-risc-v-csrs-by-using-gcc-c-code
__attribute__((always_inline)) inline uint32_t csr_mstatus_read(void){
    uint32_t result;
    asm volatile ("csrr %0, mstatus" : "=r"(result));
    return result;
}

__attribute__((always_inline)) inline void csr_mstatus_write(uint32_t val){
    asm volatile ("csrw mstatus, %0" : : "r"(val));
}

__attribute__((always_inline)) inline void csr_write_mie(uint32_t val){
    asm volatile ("csrw mie, %0" : : "r"(val));
}

__attribute__((always_inline)) inline void csr_enable_interrupts(void){
    asm volatile ("csrsi mstatus, 0x8");
}

__attribute__((always_inline)) inline void csr_disable_interrupts(void){
    asm volatile ("csrci mstatus, 0x8");
}

inline uint64_t readMachineTime()
{
#ifdef VIRT_CLINT
    uint32_t MTIME_LOW_ADDR = 0x0200bff8;
    uint32_t MTIME_HIGH_ADDR = 0x0200bffC;
#else
    uint32_t MTIME_LOW_ADDR = 0x40000008;
    uint32_t MTIME_HIGH_ADDR = 0x4000000C;
#endif
    
    uint64_t h1 = (*((volatile uint32_t *)MTIME_HIGH_ADDR));
    uint64_t l1 = (*((volatile uint32_t *)MTIME_LOW_ADDR));
    uint64_t h2 = (*((volatile uint32_t *)MTIME_HIGH_ADDR));
    uint64_t l2 = (*((volatile uint32_t *)MTIME_LOW_ADDR));
    if(h1 == h2) return ((h1 << 32) | l1);
    return (h2 << 32) | l2;
}

inline void increaseTimeCompare(uint32_t timecmp_step) {
#ifdef VIRT_CLINT
    volatile uint32_t * MTIMECMP_LOW_ADDR = ((volatile uint32_t *)0x02004000);
    volatile uint32_t * MTIMECMP_HIGH_ADDR = ((volatile uint32_t *)0x02004004);
    
#else
    volatile uint32_t * MTIMECMP_LOW_ADDR = ((volatile uint32_t *)0x40000010);
    volatile uint32_t * MTIMECMP_HIGH_ADDR = ((volatile uint32_t *)0x40000014);
#endif

    // uint64_t timecmp_step = 100;
    uint64_t curr_timer = readMachineTime();
    uint64_t NewCompare = (((uint64_t)*MTIMECMP_HIGH_ADDR) << 32) | *MTIMECMP_LOW_ADDR;
    if(NewCompare <= curr_timer)
    {
        uint64_t diff = curr_timer - NewCompare;
        uint64_t mod_diff = diff % timecmp_step;
        NewCompare = curr_timer + timecmp_step - mod_diff;
    }
        
    *MTIMECMP_LOW_ADDR = NewCompare;
    *MTIMECMP_HIGH_ADDR = NewCompare >> 32;

}

int line_printf(int idx, const char *format, ...);

int32_t *nestCriticalCount();

#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // _UTILS_H_
