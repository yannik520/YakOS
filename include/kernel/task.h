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

#ifndef _TASK_H_
#define _TASK_H_

#include <kernel/list.h>
#include <compiler.h>

#define MAX_PRIORITY	 8

#define set_task_state(tsk, state_value)		\
	do { (tsk)->state = (state_value); } while (0)
#define set_current_state(state_value)			\
	do { current_task->state = (state_value); } while (0)


enum task_state {                // Task state values
        
        RUNNING    = 0,          // Task is runnable or running
        SLEEPING   = 1,          // Task is waiting for something to happen
        SUSPENDED  = 2,          // Suspend count is non-zero
	READY      = 3,
        CREATING   = 4,          // Task is being created
	BLOCKED    = 5,
        EXITED     = 8,         // Task has exited
};


typedef int (*task_routine)(void *arg);

typedef struct task {
	struct list_head list;
	unsigned int sp;
	
	unsigned int priority;
	enum task_state state;

	void *stack;
	int stack_size;

	task_routine entry;
	void *args;

	int ret;

	char name[32];
} task_t;

extern int critical_section_count;

void initial_task_func(void);
task_t *task_alloc(char *name, int stack_size, unsigned int priority);
void task_free(task_t *task);
int task_create(task_t *task, task_routine entry, void *args);
void task_schedule(void);
void task_sleep(unsigned long delay);
void task_create_init(void);
void task_init(void);
void task_exit(int retcode);

void arch_enable_ints(void);
void arch_disable_ints(void);

static __always_inline void enter_critical_section(void)
{
	if (critical_section_count == 0)
	{
		arch_disable_ints();
	}
	critical_section_count++;
}

static __always_inline void exit_critical_section(void)
{
	critical_section_count--;
	if (critical_section_count == 0)
	{
		arch_enable_ints();
	}
}

#endif
