/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
     svc mode -------------> irq mode  ---------------> svc mode   <------>  context_switch
   +----------+	    	   +----------+		      +----------+
   |    R0    |	    	   |    R0    |		      |    R0    |
   +----------+	    	   +----------+		      +----------+
   |    R1    |	    	   |    R1    |		      |    R1    |
   +----------+	       	   +----------+		      +----------+
   |    R2    |	    	   |    R2    |		      |    R2    |
   +----------+	    	   +----------+		      +----------+
   |    R3    |	    	   |    R3    |		      |    R3    |
   +----------+     	   +----------+	  	      +----------+ <--------> +----------+
   |    R4    |        	   |    R4    |	 +------------>    R4    |     	      |    R4    |
   +----------+     	   +----------+	 | LR_irq-4   +----------+	      +----------+
   |    R5    |     	   |    R5    |	 |  +--------->    R5    |	      |    R5    |
   +----------+     	   +----------+	 |  |         +----------+	      +----------+
   |    R6    |     	   |    R6    |	 |  |  +------>    R6    |	      |    R6    |
   +----------+     	   +----------+	 |  |  |      +----------+	      +----------+
   |    R7    |     	   |    R7    |	 |  |  |      |    R7    |	      |    R7    |
   +----------+     	   +----------+	 |  |  |      +----------+	      +----------+
   |    R8    |     	   |    R8    |  |  |  |      |    R8    |	      |    R8    |
   +----------+     	   +----------+  |  |  |      +----------+	      +----------+
   |    R9    |     	   |    R9    |  |  |  |      |    R9    |	      |    R9    |
   +----------+     	   +----------+  |  |  |      +----------+	      +----------+
   |    R10   |     	   |    R10   |  |  |  |      |    R10   |	      |    R10   |
   +----------+     	   +----------+  |  |  |      +----------+	      +----------+
   |    R11   |     	   |    R11   |  |  |  |      |    R11   |	      |    R11   |
   +----------+     	   +----------+  |  |  |      +----------+	      +----------+
   |    R12   |     	   |    R12   |  |  |  |      |    R12   |	      |          |
   +----------+        	   +----------+  |  |  |      +----------+     	      +----------+
   |    SP_svc|	       	   |    SP_irq+--+  |  |      |    SP_svc| 	      |          |
   +----------+	      	   +----------+	    |  |      +----------+ 	      +----------+
   |    LR_svc|	save to	/-->    LR_irq+-----+  |      |    LR_svc| 	      |    LR_svc|
   +----------+   /-----   +----------+	       |      +----------+ <--------> +----------+
   |    PC    +---     	   |    PC    |	       |      |    PC    |
   +----------+        	   +----------+	       |      +----------+
   +----------+	       	   +----------+	       |      +----------+
   |    CPSR  +--- save to |    CPSR  |	       |      |    CPSR  |
   +----------+	  \-----   +----------+	       |      +----------+
   |  SPSR_svc|	        \--> SPSR_irq +--------+      | SPSR_svc |
   +----------+	       	   +----------+	       	      +----------+

*/
#include <linkage.h>
#include <asm.h>

.macro save_regs
	stmfd 	sp!, { r0-r12, r14 }
	sub	sp, sp, #12
	mov	r0, sp
	mrs	r1, spsr
	stmia	r0, { r1, r13-r14 }^
.endm

ENTRY(arm_save_mode_regs)
	mrs	r1, cpsr

	bic	r2, r1, #0x1f	/* clear the mode bits */

	orr     r3, r2, #0x11	/* fiq */
	msr     cpsr_c, r3
	str	r13, [r0], #4
	str	r14, [r0], #4

	orr     r3, r2, #0x12	/* irq */
	msr     cpsr_c, r3
	str	r13, [r0], #4
	str	r14, [r0], #4

	orr     r3, r2, #0x13	/* svc */
	msr     cpsr_c, r3
	str	r13, [r0], #4
	str	r14, [r0], #4

	orr     r3, r2, #0x17	/* abt */
	msr     cpsr_c, r3
	str	r13, [r0], #4
	str	r14, [r0], #4

	orr     r3, r2, #0x1b	/* und */
	msr     cpsr_c, r3
	str	r13, [r0], #4
	str	r14, [r0], #4

	orr     r3, r2, #0x1f	/* sys */
	msr     cpsr_c, r3
	str	r13, [r0], #4
	str	r14, [r0], #4

	msr	cpsr_c, r1

	bx	lr

ENTRY(arm_irq)
	/* save r4-r6 and use as a temporary place to save while we switch into supervisor mode */
	stmia	r13, { r4-r6 }
	mov	r4, r13
	sub	r5, lr, #4
	mrs	r6, spsr

	/* move into supervisor mode. irq/fiq disabled */
	msr	cpsr_c, #(3<<6 | 0x13)

	/* save the return address */
	stmfd	sp!, { r5 }

	/* save C trashed regs, supervisor lr, based on atpcs,
	   r0-r3 used to save parameter passed to sub-routine,
	   r12 use to save sp temperaly, lr use to back from
	   sub-routine.*/
	stmfd	sp!, { r0-r3, r12, lr }

	/* save spsr */
	stmfd	sp!, { r6 }

	/* restore r4-r6 */
	ldmia	r4, { r4-r6 }

	/* increment the global critical section count */
	ldr     r1, =critical_section_count
	ldr     r0, [r1]
	add     r0, r0, #1
	str     r0, [r1]

	/* call into higher level code */
	mov	r0, sp /* iframe */
	bl	platform_irq

	/* reschedule if the handler returns nonzero */
	cmp     r0, #0
	blne   	task_schedule

	/* decrement the global critical section count */
	ldr     r1, =critical_section_count
	ldr     r0, [r1]
	sub     r0, r0, #1
	str     r0, [r1]

	/* restore spsr */
	ldmfd	sp!, { r0 }
	msr     spsr_cxsf, r0

	/* restore back to where we came from, if no pc in
	   register list, with ^ will operate user mode register
	   otherwise, with ^ will back to the mode before enter
	   int, and with state switch(copy spsr_irq to cpse) */
	ldmfd	sp!, { r0-r3, r12, lr, pc }^

ENTRY(undefined)
	save_regs
	b	arm_undefined_handler
	b	.

ENTRY(syscall)
	save_regs
	b	arm_syscall_handler
	b	.

ENTRY(prefetch_abort)
	save_regs
	b	arm_prefetch_abort_handler
	b	.

ENTRY(data_abort)
	save_regs
	b	arm_data_abort_handler
	b	.

ENTRY(reserved)
	ldr	r0, =reserved_str
	bl	puts
	b	.

ENTRY(irq)
	b	arm_irq

ENTRY(fiq)
	ldr	r0, =fiq_str
	bl	puts
	b	.
.ltorg

reserved_str:
	.ascii	"reserved exception!\n\0"
	.align

fiq_str:
	.ascii	"Fiq occured!\n\0"
	.align

.ltorg

.bss
.align 2
ENTRY(irq_save_spot)
	.word	0	/* r4 */
	.word	0	/* r5 */
	.word	0	/* r6 */
.ltorg

DATA(abort_stack)
	.skip 2048
DATA(abort_stack_top)

DATA(undefined_stack)
	.skip 2048
DATA(undefined_stack_top)
