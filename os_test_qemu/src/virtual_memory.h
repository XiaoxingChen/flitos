#if !defined(_VIRTUAL_MEMORY_H_)
#define _VIRTUAL_MEMORY_H_

// reference: 
// [1] https://github.com/riscv-software-src/riscv-pk/blob/master/machine/minit.c#L224
// [2] https://jborza.com/post/2021-04-04-riscv-supervisor-mode/
inline void setup_pmp(void)
{
  // Set up a PMP to permit access to all of memory.
  // Ignore the illegal-instruction trap if PMPs aren't supported.

  uint8_t PMP_R =     0x01;
  uint8_t PMP_W =     0x02;
  uint8_t PMP_X =     0x04;
//   uint8_t PMP_A =     0x18;
//   uint8_t PMP_L =     0x80;

//   uint8_t PMP_TOR =   0x08;
//   uint8_t PMP_NA4 =   0x10;
  uint8_t PMP_NAPOT = 0x18;

  uintptr_t pmpc = PMP_NAPOT | PMP_R | PMP_W | PMP_X;
  asm volatile ("csrw pmpaddr0, %1\n\t"
                "csrw pmpcfg0, %0\n\t"
                : : "r" (pmpc), "r" (-1UL));
}

inline void switchToSupervisorMode()
{
    asm volatile (
        "li t0, (1 << 11)\n\t"
        "csrw mstatus, t0\n\t"
        "la t1, kmain\n\t"
        "csrw mepc, t1\n\t"
        "mret\n\t"
        : : );
}
#endif // _VIRTUAL_MEMORY_H_
