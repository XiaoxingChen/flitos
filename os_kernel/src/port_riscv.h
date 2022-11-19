#if !defined(_PORT_RISCV_H_)
#define _PORT_RISCV_H_

#include <stdint.h>

enum ContextOffset
{
    offset_ra = 0,
    offset_sp = 1,
    offset_gp = 2,
    offset_tp = 3,
    offset_t0 = 4,
    offset_t1 = 5,
    offset_t2 = 6,
    offset_s0 = 7,
    offset_s1 = 8,
    offset_a0 = 9,
    offset_a1 = 10,
    offset_a2 = 11,
    offset_a3 = 12,
    offset_a4 = 13,
    offset_a5 = 14,
    offset_mepc = 15,
    offset_mcause = 16,
    offset_mstatus = 17,
    context_size
};


#endif // _PORT_RISCV_H_
