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
#include <arch/memmap.h>
#include <arch/interrupts.h>
#include <arch/chip_regs.h>
#include <kernel/task.h>
#include <kernel/reg.h>
#include <kernel/printk.h>

struct arm_iframe {
	unsigned int spsr;
	unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r12;
	unsigned int lr;
	unsigned int pc;
};

struct int_handler_struct {
	int_handler handler;
	void *arg;
};

static struct int_handler_struct int_handler_table[INTNR_IRQ_END];

void platform_init_interrupts(void)
{
	writel(~0, ioaddr_intc(REG_INTC_INTENCLEAR));
	writel(0, ioaddr_intc(REG_INTC_INTSELECT));
	writel(~0, ioaddr_intc(REG_INTC_SOFTINTCLEAR));
	writel(1, ioaddr_intc(REG_INTC_SUPERPRIV_PROT));
}

int mask_interrupt(unsigned int vector)
{
	if (vector > INTNR_IRQ_END)
		return -1;

	vector -= INTNR_IRQ_START;
	enter_critical_section();
	writel(1<<vector, ioaddr_intc(REG_INTC_INTENCLEAR));
	exit_critical_section();

	return 0;
}

int unmask_interrupt(unsigned int vector)
{
	if (vector > INTNR_IRQ_END)
		return -1;

	vector -= INTNR_IRQ_START;
	enter_critical_section();
	writel(1<<vector, ioaddr_intc(REG_INTC_INTENABLE));
	exit_critical_section();

	return 0;
}

handler_return platform_irq(struct arm_iframe *frame)
{
	unsigned int irq_num = 0;
	// get the current irq status
	unsigned int irq_status = readl(ioaddr_intc(REG_INTC_IRQSTATUS));

	while((irq_status & 0x1) == 0)
	{
		irq_status >>= 1;
		irq_num ++;
	}

	// deliver the interrupt
	handler_return ret; 

	ret = INT_NO_RESCHEDULE;
	if (int_handler_table[irq_num].handler)
		ret = int_handler_table[irq_num].handler(int_handler_table[irq_num].arg);

	return ret;
}

void platform_fiq(struct arm_iframe *frame)
{
	printk("FIQ: unimplemented\n");
}

void register_int_handler(unsigned int vector, int_handler handler, void *arg)
{
	if (vector > INTNR_IRQ_END)
		printk("register_int_handler: vector out of range %d\n", vector);

	enter_critical_section();

	int_handler_table[vector].handler = handler;
	int_handler_table[vector].arg = arg;

	exit_critical_section();
}
