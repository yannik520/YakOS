.global _start
_start:
	b	reset
	b 	undefined
	b	syscall
	b	prefetch_abort
	b	data_abort
	b	reserved
	b	irq
	b	fiq

reset:
	/* disable i/d cache and mmu */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(1<<15 | 1<<13 | 1<<12)
	bic	r0, r0, #(1<<2 | 1<<0)
	orr	r0, r0, #(1<<1) /* enable alignment faults */
	mcr	p15, 0, r0, c1, c0, 0

stack_setup:
	ldr	sp, =stack_top
	bl	c_entry
	b	.

.global arm_context_switch
arm_context_switch:	
	mov		ip, sp
	sub		sp, sp, #4 //skip cpsr
.global test_point
test_point:	
	stmfd		sp!, {ip, lr}
	stmfd		sp!, {r4-r10, fp, ip}
.global test_point1
test_point1:	
	mrs		r2, cpsr
	str		r2, [sp, #44] //11*4
	str		sp, [r0]
.global test_point2
test_point2:	
	
	ldr		fp, [r1]
	mrs		r0, cpsr
	orr		r0, r0, #0xc0
	msr		cpsr, r0
	ldr		r0, [fp, #44]
	msr		spsr, r0
.global test_point3
test_point3:	

	ldmfd		fp, {r4-r10, fp, ip, sp, lr}
.global test_point4
test_point4:	

	movs		pc, lr

.global undefined
undefined:
	b	.
	
.global syscall
syscall:
	b	.

.global prefetch_abort
prefetch_abort:
	b	.
	
.global data_abort
data_abort:
	b	.
	
.global reserved
reserved:
	b	.

.global irq
irq:
	b	.
	
.global fiq
fiq:
	b	.

.data
strex_spot:
	.word	0
