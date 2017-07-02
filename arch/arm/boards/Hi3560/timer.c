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
#include <arch/timer.h>
#include <kernel/task.h>
#include <kernel/reg.h>
#include <kernel/printk.h>
#include <arch/memmap.h>
#include <arch/interrupts.h>

#define DEFAULT_MPLLIN  12000000
#define DEFAULT_BUSCLK  110000000

static platform_timer_callback t_callback;
static unsigned long timer_reload = 0;

static volatile unsigned long long ticks = 0;

unsigned long pllc_to_busclk(unsigned long pllc, unsigned long fxin)
{
	unsigned long REFDIV,FBDIV;
	
	PLLC_TO_REFDIV_FBDIV(REFDIV,FBDIV,pllc); 	

	return REFDIV_FBDIV_TO_BUSCLK(REFDIV,FBDIV,fxin);
}

int is_pllc_invalid(unsigned long pllc, unsigned long fxin)
{
	
	unsigned long REFDIV,FBDIV,value;
	
	PLLC_TO_REFDIV_FBDIV(REFDIV,FBDIV,pllc); 	
	value = fxin/REFDIV*FBDIV;
	if((value>(2400*MHZ)) || (value <= 800*MHZ))
	{
		return 1;
	}
	return 0;
}


int platform_set_periodic_timer(platform_timer_callback callback, void *arg, time_t interval)
{
	unsigned long pllc;
	unsigned long busclock;

	enter_critical_section();

	t_callback = callback;

	writel(0, CFG_TIMER_VABASE + REG_TIMER_CONTROL);
	
	pllc = readl(REG_BASE_SC + REG_SC_APLLCFGSTAT1);
	if(!is_pllc_invalid(pllc, DEFAULT_MPLLIN)) {
		busclock =pllc_to_busclk(pllc, DEFAULT_MPLLIN);
		timer_reload = BUSCLK_TO_TIMER_RELOAD(busclock);
		
	}
	else
	{
		timer_reload = BUSCLK_TO_TIMER_RELOAD(DEFAULT_BUSCLK);
	}
	timer_reload = 6800000;
	writel(timer_reload, CFG_TIMER_VABASE + REG_TIMER_RELOAD);
	writel(CFG_TIMER_CONTROL, CFG_TIMER_VABASE + REG_TIMER_CONTROL);

	unmask_interrupt(CFG_TIMER_INTNR);

	exit_critical_section();

	return 0;
}

static unsigned long gettimeoffset(void)
{
	unsigned long ticks1, ticks2, status;

	ticks2 = readl(CFG_TIMER_VABASE + REG_TIMER_VALUE);
	do {
		ticks1 = ticks2;
		status = readl(ioaddr_intc(REG_INTC_RAWSTATUS));
		ticks2 = readl(CFG_TIMER_VABASE + REG_TIMER_VALUE);

	} while ((ticks2 > ticks1) || (ticks2 == 0));

	ticks1 = timer_reload - ticks2;
	//printk("ticks1=%d, ticks2=%d, status=0x%x\n", ticks1, ticks2, status);

	if(status & (1<<CFG_TIMER_INTNR))
		ticks1 += timer_reload;

	return ticks2ms(ticks1);
}

unsigned long long current_time(void)
{
	return ticks;
}

static handler_return platform_tick(void *arg)
{
	ticks += gettimeoffset();
	writel(~0, CFG_TIMER_VABASE + REG_TIMER_INTCLR);

	if (t_callback) {
		return t_callback(arg, current_time());
	} else {
		return INT_NO_RESCHEDULE;
	}
}

void platform_init_timer(void)
{
	register_int_handler(CFG_TIMER_INTNR, &platform_tick, 0);
}

