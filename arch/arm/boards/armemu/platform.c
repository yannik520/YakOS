#include "arch/text.h"
#include "arch/interrupts.h"
#include "arch/timer.h"
#include "arch/platform.h"
#include "kernel/printf.h"

void console_init(void)
{
	__console_init();
}

static enum handler_return timer_tick(void *arg, time_t now)
{
	printf("timer_tick\n");
	return INT_RESCHEDULE;
}

void platform_init(void)
{
	/* init serial port */
	console_init();
	
	/* init interrupt controller */
	platform_init_interrupts();

	/* init timmer for kernel tick */
	platform_init_timer();
	platform_set_periodic_timer(timer_tick, 0, 1000); /* 1000ms */
}
