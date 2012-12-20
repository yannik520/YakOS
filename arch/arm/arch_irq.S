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
   |    R12   |     	   |    R12   |  |  |  |      |    R12   |	      |    R12   |
   +----------+        	   +----------+  |  |  |      +----------+     	      +----------+
   |    SP_svc|	       	   |    SP_irq+--+  |  |      |    SP_svc| 	      |    SP_svc|
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

.global arm_irq
arm_irq:
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

	/* call into higher level code */
	mov	r0, sp /* iframe */
	bl	platform_irq

	/* reschedule if the handler returns nonzero */
	bl    	task_schedule

	/* restore spsr */
	ldmfd	sp!, { r0 }
	msr     spsr_cxsf, r0

	/* restore back to where we came from, if no pc in
	   register list, with ^ will operate user mode register
	   otherwise, with ^ will back to the mode before enter
	   int, and with state switch(copy spsr_irq to cpse)*/
	ldmfd	sp!, { r0-r3, r12, lr, pc }^

.bss
.align 2
.global irq_save_spot
irq_save_spot:
	.word	0	/* r4 */
	.word	0	/* r5 */
	.word	0	/* r6 */
