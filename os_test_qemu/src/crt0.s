.section .init, "ax"
.global _start
_start:
    .cfi_startproc
    .cfi_undefined ra
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop
    la sp, __stack_top
    add s0, sp, zero
    la  a5, _interrupt_handler
    csrw mtvec, a5
    jal ra, init
    nop
    jal zero, main
    .cfi_endproc

.section .text, "ax"
.global mutexInit, mutexDestroy
.global threadCreate, threadYield, threadJoin, threadSleep
  
threadCreate:
    li a5, 10
    ecall
threadYield:
    li a5, 11
    ecall  
getGlobalPointer:
    li a5, 12
    ecall
mutexInit:
    li a5, 13
    ecall
mutexDestroy:
    li a5, 14
    ecall
mutexAcquire:
    li a5, 15
    ecall
mutexRelease:
    li a5, 16
    ecall
threadJoin:
    li a5, 17
    ecall
threadSleep:
    li a5, 18
    ecall
.end
