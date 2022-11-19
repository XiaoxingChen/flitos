.macro context_snapshot output_mem
    csrw mscratch, t0
    la t0, \output_mem
    sw ra, 0*4(t0)
	sw sp, 1*4(t0)
	sw gp, 2*4(t0)
	sw tp, 3*4(t0)
	sw t1, 5*4(t0)
	sw t2, 6*4(t0)
	sw s0, 7*4(t0)
	sw s1, 8*4(t0)
	sw a0, 9*4(t0)
	sw a1, 10*4(t0)
	sw a2, 11*4(t0)
	sw a3, 12*4(t0)
	sw a4, 13*4(t0)
	sw a5, 14*4(t0)
	csrr t1, mscratch /* put t0's value into t1 */
	sw t1, 4*4(t0)

	csrr t1, mepc
	sw t1, 15*4(t0)
	csrr t1, mcause
	sw t1, 16*4(t0)
	csrr t1, mstatus
	sw t1, 17*4(t0)

    lw t1, 5*4(t0) /* recover t1 */
    csrr t0, mscratch /* recover t0 */
.endm

.macro context_project input_mem
    la t0, \input_mem

	lw t1, 4*4(t0) /* put t0 to t1*/
	csrw mscratch, t1 /* put t0 to mscratch*/

    lw ra, 0*4(t0)
	lw sp, 1*4(t0)
	lw gp, 2*4(t0)
	lw tp, 3*4(t0)
	lw t1, 5*4(t0)
	lw t2, 6*4(t0)
	lw s0, 7*4(t0)
	lw s1, 8*4(t0)
	lw a0, 9*4(t0)
	lw a1, 10*4(t0)
	lw a2, 11*4(t0)
	lw a3, 12*4(t0)
	lw a4, 13*4(t0)
	lw a5, 14*4(t0)

	csrr t0, mscratch /* put t0's value into t1 */
.endm