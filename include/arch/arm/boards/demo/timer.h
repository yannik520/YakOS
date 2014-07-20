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

#ifndef _TIMER_H_
#define _TIMER_H_

#include <arch/interrupts.h>
#include <arch/chip_regs.h>
#include <arch/memory.h>

#ifndef MHZ
#define MHZ             (1000*1000)
#endif

#ifndef HZ
#define HZ		100
#endif

//#define CFG_TIMER_VABASE	(uint32_t)phys_to_virt(REG_BASE_TIMER_01)
#define CFG_TIMER_VABASE	REG_BASE_TIMER_01
#define CFG_TIMER_CONTROL	( (1<<7) | (1<<6) | (1<<5) | (1<<1) )
#define CFG_TIMER_PRESCALE	1
#define BUSCLK_TO_TIMER_RELOAD(busclk)	(((busclk)/CFG_TIMER_PRESCALE)/HZ)
#define CFG_TIMER_INTNR		INTNR_TIMER_0

#define ticks2us(ticks) (((ticks)*((1000000/HZ) >> 2))/(timer_reload >> 2))
#define ticks2ms(ticks) (((ticks)*((1000/HZ) >> 2))/(timer_reload >> 2))

typedef unsigned long time_t;
typedef unsigned long long bigtime_t;

typedef handler_return (*platform_timer_callback)(void *arg, time_t now);

void platform_init_timer(void);
int platform_set_periodic_timer(platform_timer_callback callback, void *arg, time_t interval);
unsigned long long current_time(void);

#endif
