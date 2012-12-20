#include "arch/timer.h"
#include "kernel/task.h"
#include "kernel/reg.h"
#include "arch/memmap.h"
#include "arch/interrupts.h"

static platform_timer_callback t_callback;

int platform_set_periodic_timer(platform_timer_callback callback, void *arg, time_t interval)
{
	enter_critical_section();

	t_callback = callback;

	*REG(PIT_CLEAR) = 1;
	*REG(PIT_INTERVAL) = interval;
	*REG(PIT_START_PERIODIC) = 1;

	unmask_interrupt(INT_PIT);

	exit_critical_section();

	return 0;
}

bigtime_t current_time_hires(void)
{
	bigtime_t time;
	*REG(SYSINFO_TIME_LATCH) = 1;
	time = *REG(SYSINFO_TIME_SECS) * 1000000ULL;
	time += *REG(SYSINFO_TIME_USECS);

	return time;
}

time_t current_time(void)
{
	time_t time;
	*REG(SYSINFO_TIME_LATCH) = 1;
	time = *REG(SYSINFO_TIME_SECS) * 1000;
	time += *REG(SYSINFO_TIME_USECS) / 1000;

	return time;
}

static enum handler_return platform_tick(void *arg)
{
	*REG(PIT_CLEAR_INT) = 1;
	if (t_callback) {
		return t_callback(arg, current_time());
	} else {
		return INT_NO_RESCHEDULE;
	}
}

void platform_init_timer(void)
{
	register_int_handler(INT_PIT, &platform_tick, 0);
}

