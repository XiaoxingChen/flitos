.section .text, "ax"
.include "context_snapshot.h"
.global _interrupt_handler, context_switch, startFirstTask
.extern context_shot

context_switch:
    /*context_snapshot context_shot
    call printContextSnapshot 
    context_project context_shot*/

    /* csrci   mstatus, 0x8*/ /* disable interrupt */
    addi    sp,sp,-18*4 /* context size 18: see port_riscv.h */
    sw      ra,0*4(sp)
    sw      gp,2*4(sp)
    sw      tp,3*4(sp)
    sw      t0,4*4(sp)
    sw      t1,5*4(sp)
    sw      t2,6*4(sp)
    sw      s0,7*4(sp)
    sw      s1,8*4(sp)
    sw      a0,9*4(sp)
    sw      a1,10*4(sp)
    sw      a2,11*4(sp)
    sw      a3,12*4(sp)
    sw      a4,13*4(sp)
    sw      a5,14*4(sp)

    csrr    t0, mepc
    sw      t0,15*4(sp) /* write mepc into context */

    sw      sp,0(a0)
    mv      sp,a1

    lw      t0,15*4(sp)
    csrw    mepc, t0  /* read mepc from context, write to register */

    lw      ra,0*4(sp)
    lw      gp,2*4(sp)
    lw      tp,3*4(sp)
    lw      t0,4*4(sp)
    lw      t1,5*4(sp)
    lw      t2,6*4(sp)
    lw      s0,7*4(sp)
    lw      s1,8*4(sp)
    lw      a0,9*4(sp)
    lw      a1,10*4(sp)
    lw      a2,11*4(sp)
    lw      a3,12*4(sp)
    lw      a4,13*4(sp)
    lw      a5,14*4(sp)
    addi    sp,sp,18*4

    /*context_snapshot context_shot
    call printContextSnapshot
    context_project context_shot */
    /* csrsi   mstatus, 0x8*/ /* enable interrupt */
    ret

startFirstTask:
    mv      sp,a0
    lw      ra,0*4(sp)
    lw      gp,2*4(sp)
    lw      tp,3*4(sp)
    lw      t0,4*4(sp)
    lw      t1,5*4(sp)
    lw      t2,6*4(sp)
    lw      s0,7*4(sp)
    lw      s1,8*4(sp)
    lw      a0,9*4(sp)
    lw      a1,10*4(sp)
    lw      a2,11*4(sp)
    lw      a3,12*4(sp)
    lw      a4,13*4(sp)
    lw      a5,14*4(sp)
    addi    sp,sp,18*4
    /*csrsi   mstatus, 0x8*/ /* enable interrupt */
    ret
