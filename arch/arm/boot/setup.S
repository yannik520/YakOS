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

#include <linkage.h>

.section ".text.boot"
ENTRY(_start)
	ldr	pc, _reset
	ldr	pc, _undefined
	ldr	pc, _syscall
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _reserved
	ldr	pc, _irq
	ldr	pc, _fiq

_reset:
	.word reset
_undefined:
	.word undefined
_syscall:
	.word syscall
_prefetch_abort:
	.word prefetch_abort
_data_abort:
	.word data_abort
_reserved:
	.word reserved
_irq:
	.word arm_irq
_fiq:
	.word fiq
	
ENTRY(reset)
	/* disable i/d cache and mmu */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(1<<15 | 1<<13 | 1<<12)
	bic	r0, r0, #(1<<2 | 1<<0)
	/* enable alignment faults */
	orr	r0, r0, #(1<<1) 
	mcr	p15, 0, r0, c1, c0, 0

/* relocate exception vertor table */
relocate_vectors:
	/* save current position to r0 */
	adr	r0, _start
	mov	r1, #0x0
	/* if not start from address 0x0, relocat vertor table */
	cmp	r0, r1
	beq	stack_setup
	adr	r2, reset
copy_loop:	
	ldmia	r0!, {r3-r10}
	stmia	r1!, {r3-r10}
	cmp	r0, r2
	blo	copy_loop
	
stack_setup:
	/* for irq mode */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r1, r0, #0x12
	msr	cpsr_c, r1
	ldr	r13, =irq_save_spot
	/* for svc mode */
	orr	r1, r0, #0x13
	msr	cpsr_c, r1
	ldr	sp, =stack_top

clear_bss:
	/* clear out the bss */
	ldr		r0, =__bss_start
	ldr		r1, =_end
	mov		r2, #0
__bss_loop:
	cmp		r0, r1
	strlt		r2, [r0], #4
	blt		__bss_loop
	/* start c program */
	bl	c_entry
	b	.


ENTRY(arm_context_switch)
	push    { r4-r11, lr}

	/* save sp */
	str     sp, [r0]

	/* load new regs */
	mov     sp, r1
	pop     { r4-r11, lr}
	bx      lr
.data
strex_spot:
	.word	0
