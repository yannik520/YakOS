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
#include <arch/arm.h>
#include <kernel/printk.h>
#include <kernel/debug.h>

extern void halt(void);

static void dump_fault_frame(struct arm_fault_frame *frame)
{
	printk("r0  0x%08x r1  0x%08x r2  0x%08x r3  0x%08x\n", frame->r[0], frame->r[1], frame->r[2], frame->r[3]);
	printk("r4  0x%08x r5  0x%08x r6  0x%08x r7  0x%08x\n", frame->r[4], frame->r[5], frame->r[6], frame->r[7]);
	printk("r8  0x%08x r9  0x%08x r10 0x%08x r11 0x%08x\n", frame->r[8], frame->r[9], frame->r[10], frame->r[11]);
	printk("r12 0x%08x usp 0x%08x ulr 0x%08x pc  0x%08x\n", frame->r[12], frame->usp, frame->ulr, frame->pc);
	printk("spsr 0x%08x\n", frame->spsr);

	struct arm_mode_regs regs;
	arm_save_mode_regs(&regs);

	printk("%c%s r13 0x%08x r14 0x%08x\n", ((frame->spsr & MODE_MASK) == MODE_FIQ) ? '*' : ' ', "fiq", regs.fiq_r13, regs.fiq_r14);
	printk("%c%s r13 0x%08x r14 0x%08x\n", ((frame->spsr & MODE_MASK) == MODE_IRQ) ? '*' : ' ', "irq", regs.irq_r13, regs.irq_r14);
	printk("%c%s r13 0x%08x r14 0x%08x\n", ((frame->spsr & MODE_MASK) == MODE_SVC) ? '*' : ' ', "svc", regs.svc_r13, regs.svc_r14);
	printk("%c%s r13 0x%08x r14 0x%08x\n", ((frame->spsr & MODE_MASK) == MODE_UND) ? '*' : ' ', "und", regs.und_r13, regs.und_r14);
	printk("%c%s r13 0x%08x r14 0x%08x\n", ((frame->spsr & MODE_MASK) == MODE_SYS) ? '*' : ' ', "sys", regs.sys_r13, regs.sys_r14);

	// dump the bottom of the current stack
	addr_t stack;
	switch (frame->spsr & MODE_MASK) {
		case MODE_FIQ:
			stack = regs.fiq_r13;
			break;
		case MODE_IRQ:
			stack = regs.irq_r13;
			break;
		case MODE_SVC:
			stack = regs.svc_r13;
			break;
		case MODE_UND:
			stack = regs.und_r13;
			break;
		case MODE_SYS:
			stack = regs.sys_r13;
			break;
		default:
			stack = 0;
	}

	if (stack != 0) {
		printk("bottom of stack at 0x%08x:\n", (unsigned int)stack);
		hexdump((void *)stack, 128);
	}
}


static void exception_die(struct arm_fault_frame *frame, int pc_off, const char *msg)
{
	frame->pc += pc_off;
	printk(msg);
	dump_fault_frame(frame);

	halt();
	for (;;);
}

void arm_syscall_handler(struct arm_fault_frame *frame)
{
	exception_die(frame, -4, "Unhandled syscall\n");
}

void arm_undefined_handler(struct arm_fault_frame *frame)
{
	exception_die(frame, -4, "Undefined abort!\n");
}

void arm_data_abort_handler(struct arm_fault_frame *frame)
{
	exception_die(frame, -8, "Data abort!\n");
}

void arm_prefetch_abort_handler(struct arm_fault_frame *frame)
{
	exception_die(frame, -4, "Prefetch abort!\n");
}
