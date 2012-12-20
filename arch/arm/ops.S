.text

/* void arch_enable_ints(void); */
.global arch_enable_ints
arch_enable_ints:	
	mrs	r0, cpsr
	bic	r0, r0, #(1<<7)		/* clear the I bit */
	msr	cpsr_c, r0
	bx	lr

/* void arch_disable_ints(void); */
.global arch_disable_ints
arch_disable_ints:	
	mrs	r0, cpsr
	orr	r0, r0, #(1<<7)
	msr	cpsr_c, r0
	bx	lr
