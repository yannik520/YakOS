.global arm_irq
arm_irq:
	/* XXX only deals with interrupting supervisor mode */

	/* save r4-r6 and use as a temporary place to save while we switch into supervisor mode */
	stmia	r13, { r4-r6 }
	mov		r4, r13
	sub		r5, lr, #4
	mrs		r6, spsr

	/* move into supervisor mode. irq/fiq disabled */
	msr	cpsr_c, #(3<<6 | 0x13)

	/* save the return address */
	stmfd	sp!, { r5 }

	/* save C trashed regs, supervisor lr */
	stmfd	sp!, { r0-r3, r12, lr }

	/* save spsr */
	stmfd	sp!, { r6 }

	/* restore r4-r6 */
	ldmia	r4, { r4-r6 }

	/* call into higher level code */
	mov	r0, sp /* iframe */
	bl	platform_irq

	/* reschedule if the handler returns nonzero */
	bl    task_schedule

	/* restore spsr */
	ldmfd	sp!, { r0 }
	msr     spsr_cxsf, r0

	/* restore back to where we came from */
	ldmfd	sp!, { r0-r3, r12, lr, pc }^
	
.bss
.align 2
.global irq_save_spot
irq_save_spot:
	.word	0	/* r4 */
	.word	0	/* r5 */
	.word	0	/* r6 */
