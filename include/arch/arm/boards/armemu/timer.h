#ifndef _TIMER_H_
#define _TIMER_H_

#include "arch/interrupts.h"

typedef unsigned long time_t;
typedef unsigned long long bigtime_t;

typedef handler_return (*platform_timer_callback)(void *arg, time_t now);

void platform_init_timer(void);
int platform_set_periodic_timer(platform_timer_callback callback, void *arg, time_t interval);

#endif
