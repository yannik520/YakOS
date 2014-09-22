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
#include <string.h>
#include <kernel/task.h>
#include <arch/arch_task.h>
#include <arch/cpu.h>

#define ALIGNTO(x, y)    ((x) &= ~((y) - 1))

struct context_switch_frame {
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int lr;
};

extern void arm_context_switch(unsigned long, unsigned long);

void arch_task_initialize(task_t *t)
{

	unsigned int stack_top = (unsigned int)t->stack + t->stack_size;

	/* align to 8byte */
	ALIGNTO(stack_top, 8);

	struct context_switch_frame *frame = (struct context_switch_frame *)stack_top;
	frame--;

	memset(frame, 0, sizeof(*frame));

	frame->lr = (unsigned int)&initial_task_func;
	t->sp	  = (unsigned int)(frame);
}

void arch_context_switch(task_t *oldtask, task_t *newtask)
{
	/* printk("oldtask->sp=0x%x, newtask->sp=0x%x\n", */
	/*        oldtask->sp, newtask->sp); */
	cpu_switch_mm((unsigned long)newtask->mm.pgd);
	arm_context_switch(&oldtask->sp, newtask->sp);
}
