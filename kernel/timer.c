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
#include <kernel/list.h>
#include <arch/timer.h>
#include <arch/interrupts.h>
#include <kernel/timer.h>
#include <kernel/task.h>
#include <kernel/printk.h>

//#define DEBUG    1
#include <kernel/debug.h>

struct list_head timer_list;

static void dump_timers(void)
{
	timer_t *timer;

	printk("all timers:\n");
	list_for_each_entry(timer, &timer_list, entry)
	{
		printk(" %s timer: 0x%x %d ",
		       ((task_t *)(timer->arg))->name,
		       (unsigned int)timer, timer->expired_time);
	}
	printk("\n");
}

static void timer_list_add(timer_t *timer)
{
	timer_t *iterator;

	dbg("Etime:%d\n", timer->expired_time);
	#ifdef DEBUG
	dump_timers();
	#endif

	if (list_empty(&timer_list))
	{
		list_add(&timer->entry, &timer_list);
		return;
	}

	list_for_each_entry(iterator, &timer_list, entry)
	{
		dbg("%d\n", iterator->expired_time);
		if (iterator->expired_time < timer->expired_time)
		{
			list_add(&timer->entry, &iterator->entry);
			return;
		}
	}
	list_add_tail(&timer->entry, &timer_list);

	#ifdef DEBUG
	dump_timers();
	#endif
}

static void timer_add(timer_t *timer, unsigned int delay_time, unsigned int period, timer_function function, void *arg)
{
	unsigned long long now;

	if (!list_empty(&timer->entry))
	{
		error("timer has been added\n");
	}
	
	now = current_time();
	timer->expired_time = now + delay_time;
	timer->periodic_time = period;
	timer->function = function;
	timer->arg = arg;

	enter_critical_section();
	timer_list_add(timer);
	exit_critical_section();
}

void oneshot_timer_add(timer_t *timer, unsigned long delay, timer_function function, void *arg)
{
	if (0 == delay)
	{
		delay = 1;
	}

	timer_add(timer, delay, 0, function, arg);
}

void periodic_timer_add(timer_t *timer, unsigned long period, timer_function function, void *arg)
{
	if (0 == period)
	{
		period = 1;
	}

	timer_add(timer, period, period, function, arg);
}

void timer_delete(timer_t *timer)
{
	enter_critical_section();
	
	if (!list_empty(&timer->entry))
	{
		list_del_init(&timer->entry);
	}
	timer->periodic_time = 0;
	timer->expired_time = 0;
	timer->function = NULL;
	timer->arg = NULL;
	
	exit_critical_section();
}

enum handler_return timer_tick(void *arg, bigtime_t now)
{
	timer_t *timer;
	timer_t *timer_tmp;
	enum handler_return ret = INT_NO_RESCHEDULE;
	static first_time       = 1;

	if (first_time)
	{
	    first_time = 0;
	    return INT_RESCHEDULE;
	}

	dbg("now=%d\n", now);

	#ifdef DEBUG
	dump_timers();
	#endif

	list_for_each_entry_safe(timer, timer_tmp, &timer_list, entry)
	{
		unsigned long expired_time  = timer->expired_time;
		unsigned long periodic_time = timer->periodic_time;

		/* if (&timer_tmp->entry == &timer_list) */
		/* { */
		/* 	break; */
		/* } */

		if ((signed long)(expired_time - now) > 0)
		{
			break;
		}

		//list_del_init(&timer->entry);
		list_del(&timer->entry);

		
		if (timer->function(timer, now, timer->arg) == INT_RESCHEDULE)
		{
			ret = INT_RESCHEDULE;
			return ret;
		}

		if (periodic_time && list_empty(&timer->entry))
		{
			timer->expired_time = now + periodic_time;
			timer_list_add(timer);
			
		}

	}

	return ret;
}

void timer_init(void)
{
	INIT_LIST_HEAD(&timer_list);

	/* register for a periodic timer tick */
	platform_set_periodic_timer(timer_tick, NULL, 10); /* 10ms */
}
