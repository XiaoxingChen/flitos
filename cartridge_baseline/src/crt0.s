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
    jal ra, init
    nop
    jal zero, main
    .cfi_endproc
    

.section .text, "ax"
.global getTicks, getStatus, getVideoInterruptSeq, hookFunction, getCmdInterruptSeq, registerHandler, getGlobalPointer
.global writeTargetMem, writeTarget
.global mutexInit, mutexDestroy
.global threadCreate, threadYield, threadJoin

getTicks:
    li a5, 0
    ecall
getStatus:
    li a5, 1
    ecall
getVideoInterruptSeq:
    li a5, 2
    ecall  
registerHandler:
    li a5, 5
    ecall
writeTargetMem:
    li a5, 6
    ecall
writeTarget:
    li a5, 7
    ecall
hookFunction:
    li a5, 8
    ecall
getCmdInterruptSeq:
    li a5, 9
    ecall  
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
.end