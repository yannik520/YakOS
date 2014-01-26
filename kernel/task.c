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
#include <string.h>
#include <kernel/task.h>
#include <kernel/printk.h>
#include <kernel/malloc.h>
#include <kernel/type.h>
#include <kernel/timer.h>
#include <arch/arch_task.h>
#include <kernel/sched.h>

//#define DEBUG           1
#include <kernel/debug.h>

#define STACK_DEF_SIZE    (0x2000)
#define DEFAULT_PRIORITY  (MAX_PRIORITY - 2)
#define INIT_TASK_NAME    "init"

task_t				*current_task;
task_t				*next_task;
int				 critical_section_count = 0;
extern struct sched_class	*scheduler;

void initial_task_func(void)
{
	int ret;

	exit_critical_section();
	ret = current_task->entry(current_task->args);
	task_exit(ret);
}

task_t *task_alloc(char *name, int stack_size, unsigned int priority)
{
	task_t *task;

	if (NULL == name || priority >= MAX_PRIORITY)
	{
		return NULL;
	}

	task = (task_t *)kmalloc(sizeof(task_t));
	if (task == (task_t *)0)
	{
		printk("Alloc task_t error!\n");
		return NULL;
	}
	memset(task, 0, sizeof(task_t));
	memcpy(task->name, name, strlen(name) + 1);
	task->stack_size = stack_size;
	task->priority   = priority;

	return task;
}

void task_free(task_t *task)
{
	if (NULL == task)
	{
		return;
	}
	
	kfree(task->stack);
	kfree(task);
}

int task_create(task_t *task, task_routine entry, void *args)
{
	unsigned int     *stack_addr;

	if (0 == task->stack_size)
	{
		task->stack_size = STACK_DEF_SIZE;
	}

	stack_addr = (unsigned int *)kmalloc(task->stack_size);
	if (stack_addr == NULL)
	{
		return -1;
	}

	task->stack      = stack_addr;
	task->entry      = entry;
	task->args	 = args;
	task->state      = CREATING;

	arch_task_initialize(task);
	INIT_LIST_HEAD(&task->list);

	scheduler->enqueue_task(task, 0);

	return 0;
}

void task_schedule(void)
{
	task_t              *new_task;
	task_t              *old_task = current_task;

	new_task = scheduler->pick_next_task();

	if ((NULL == new_task) || (old_task == new_task))
	{
		return;
	}

	arch_context_switch(old_task, new_task);
}

static enum handler_return task_sleep_function(timer_t *timer, unsigned long now, void *arg)
{
	task_t *t = (task_t *)arg;

	DBG("%s wakeup\n", t->name);

	t->state = READY;

	enter_critical_section();

	scheduler->enqueue_task(t, 0);
	kfree(timer);

	exit_critical_section();

	return INT_RESCHEDULE;
}

void task_sleep(unsigned long delay)
{
	timer_t         *timer;
	
	DBG("start sleep\n");

	#ifdef DEBUG
	scheduler->dump();
	#endif
	
	timer = (timer_t *)kmalloc(sizeof(*timer));
	if (NULL == timer)
	{
		ERROR("%s: alloc timer failled\n");
	}
	
	init_timer_value(timer);

	enter_critical_section();

	oneshot_timer_add(timer, delay, task_sleep_function, (void *)current_task);
	current_task->state = SLEEPING;

	scheduler->dequeue_task(current_task, 0);

	task_schedule();
	exit_critical_section();
	
}

void task_create_init(void)
{
	unsigned int	*stack_addr;
	task_t		*init;

	init = (task_t *)kmalloc(sizeof(task_t));
	if (init == (task_t *)0)
	{
		ERROR("Alloc task_t error!\n");
		return;
	}

	stack_addr = (unsigned int *)kmalloc(STACK_DEF_SIZE);
	if (stack_addr == NULL)
	{
		kfree(init);
		return;
	}

	init->sp         = (unsigned int)stack_addr + STACK_DEF_SIZE;
	init->stack_size = STACK_DEF_SIZE;
	init->priority	 = MAX_PRIORITY-1;
	init->state      = CREATING;
	memcpy(init->name, INIT_TASK_NAME, strlen(INIT_TASK_NAME) + 1);

	INIT_LIST_HEAD(&init->list);

	scheduler->enqueue_task(init, 0);

	current_task = init;
}

void task_init(void)
{
	sched_init();
}

void task_exit(int retcode)
{
	struct list_head    *iterator;

	enter_critical_section();

	current_task->state = EXITED;
	current_task->ret   = retcode;

	#ifdef DEBUG
	scheduler->dump();
	#endif

	scheduler->dequeue_task(current_task, 0);

	task_schedule();
}
