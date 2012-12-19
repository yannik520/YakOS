.section ".text.boot"
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
	mov		r12, sp
.global test_point
test_point:	
	stmfd		sp!, {r12, lr}
	stmfd		sp!, {r4-r10, r11, r12}
.global test_point1
test_point1:	
	str		sp, [r0]
.global test_point2
test_point2:	
	
	ldr		r12, [r1]
.global test_point3
test_point3:	
	ldmfd		r12, {r4-r10, r11, r12, sp, lr}
.global test_point4
test_point4:	
	mov		pc, lr

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
