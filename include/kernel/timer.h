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
#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

#include <compiler.h>
#include <kernel/list.h>
#include <kernel/types.h>
#include <arch/interrupts.h>

struct timer;
typedef enum handler_return (*timer_function)(struct timer *timer, unsigned long current_time, void *arg);

typedef volatile struct timer {
	struct list_head	entry;
	unsigned long		expired_time;
	unsigned long		periodic_time;
	timer_function		function;
	void *arg;
}timer_t;

#define TIMER_INITIALIZER(_function, _expires, _periodic,  _data) {	\
		.function      = (_function),	\
		.expired_time  = (_expires),	\
    		.periodic_time = (_periodic),	\
		.arg	       = (_data),	\
	}

static __always_inline void init_timer_value(timer_t *t)
{
	t->expired_time = 0;
	t->periodic_time = 0;
	t->function = NULL;
	t->arg = NULL;
	INIT_LIST_HEAD((struct list_head *)&t->entry);
}

unsigned long long current_time(void);
void oneshot_timer_add(timer_t *timer, unsigned long delay, timer_function function, void *arg);
void periodic_timer_add(timer_t *timer, unsigned long period, timer_function function, void *arg);
void timer_delete(timer_t *timer);
void timer_init(void);

#endif
