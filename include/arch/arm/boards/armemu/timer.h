#ifndef _TIMER_H_
#define _TIMER_H_

typedef unsigned long time_t;
typedef unsigned long long bigtime_t;

typedef enum handler_return (*platform_timer_callback)(void *arg, time_t now);

int platform_set_periodic_timer(platform_timer_callback callback, void *arg, time_t interval);

#endif
