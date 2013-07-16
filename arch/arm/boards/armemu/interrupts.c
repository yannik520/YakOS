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
#include "arch/memmap.h"
#include "arch/interrupts.h"
#include "kernel/task.h"
#include "kernel/reg.h"
#include "kernel/printf.h"

struct int_handler_struct {
	int_handler handler;
	void *arg;
};

static struct int_handler_struct int_handler_table[PIC_MAX_INT];

void platform_init_interrupts(void)
{
	// mask all the interrupts
	*REG32(PIC_MASK_LATCH) = 0xffffffff;
}

int mask_interrupt(unsigned int vector)
{
	if (vector >= PIC_MAX_INT)
		return -1;

//	printf("%s: vector %d\n", __PRETTY_FUNCTION__, vector);

	enter_critical_section();

	*REG32(PIC_MASK_LATCH) = 1 << vector;

	exit_critical_section();

	return 0;
}

int unmask_interrupt(unsigned int vector)
{
	if (vector >= PIC_MAX_INT)
		return -1;

//	printf("%s: vector %d\n", __PRETTY_FUNCTION__, vector);

	enter_critical_section();

	*REG32(PIC_UNMASK_LATCH) = 1 << vector;

	exit_critical_section();

	return 0;
}

handler_return platform_irq(struct arm_iframe *frame)
{
	// get the current vector
	unsigned int vector = *REG32(PIC_CURRENT_NUM);
	if (vector == 0xffffffff)
		return INT_NO_RESCHEDULE;

//	printf("platform_irq: spsr 0x%x, pc 0x%x, currthread %p, vector %d\n", frame->spsr, frame->pc, current_thread, vector);

	// deliver the interrupt
	handler_return ret; 

	ret = INT_NO_RESCHEDULE;
	if (int_handler_table[vector].handler)
		ret = int_handler_table[vector].handler(int_handler_table[vector].arg);

//	printf("platform_irq: exit %d\n", ret);

	return ret;
}

void platform_fiq(struct arm_iframe *frame)
{
	printf("FIQ: unimplemented\n");
}

void register_int_handler(unsigned int vector, int_handler handler, void *arg)
{
	if (vector >= PIC_MAX_INT)
		printf("register_int_handler: vector out of range %d\n", vector);

	enter_critical_section();

	int_handler_table[vector].handler = handler;
	int_handler_table[vector].arg = arg;

	exit_critical_section();
}
