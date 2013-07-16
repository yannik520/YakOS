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
#include "kernel/task.h"
#include "kernel/printf.h"
#include "kernel/malloc.h"
#include "kernel/type.h"
#include "arch/arch_task.h"

#define STACK_DEF_SIZE    (0x800)
#define DEFAULT_PRIORITY  (MAX_PRIORITY - 2)

struct list_head	 all_task[MAX_PRIORITY];
unsigned int		 task_bitmap;
task_t			*current_task;

void initial_task_func(void)
{
	int ret;
	
	exit_critical_section();
	ret = current_task->entry(current_task->args);
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
		printf("Alloc task_t error!\n");
		return;
	}
	memset(task, 0, sizeof(task_t));
	memcpy(task->name, name, strlen(name));
	task->stack_size = stack_size;
	task->priority = priority;

	return task;
}

void task_free(task_t *task)
{
	if (NULL == task)
	{
		return NULL;
	}
	
	kfree(task);
}

int task_create(task_t *task, task_routine entry, void *args)
{
	unsigned int	*stack_addr;

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

	enter_critical_section();
	list_add_tail(&task->list, &all_task[task->priority]);
	exit_critical_section();

	task_bitmap |= 1 << task->priority;

	return 0;
}

void task_schedule(void)
{
	struct list_head    *list     = &current_task->list;
	task_t              *old_task = current_task;
	task_t              *new_task;
	int                  i;

	if (list_empty(list) || list_is_last(list, &all_task[current_task->priority]))
	{

		for (i=0; i<MAX_PRIORITY; i++)
		{
			if ((task_bitmap >> i) & 1)
			{
				break;
			}
		}
		
		if (i < MAX_PRIORITY)
		{
			new_task = (task_t *)all_task[i].next;
		}
		else
		{
			goto out;
		}
		
	}
	else
	{
		new_task = (task_t *)current_task->list.next;
	}
       
	new_task->state = RUNNING;

	current_task = new_task;

	arch_context_switch(old_task, new_task);
out:
	printf("task_schedule end!\n");
}

void task_create_init(void)
{
	unsigned int	*stack_addr;
	task_t		*init;

	init = (task_t *)kmalloc(sizeof(task_t));
	if (init == (task_t *)0)
	{
		printf("Alloc task_t error!\n");
		return;
	}

	stack_addr = (unsigned int *)kmalloc(STACK_DEF_SIZE);
	if (stack_addr == NULL)
	{
		kfree(init);
		return;
	}

	init->sp = (unsigned int)stack_addr + STACK_DEF_SIZE;
	init->priority = MAX_PRIORITY-1;
	INIT_LIST_HEAD(&init->list);
	list_add_tail(&init->list, &all_task[init->priority]);
	current_task = init;
}

void task_init(void)
{
	int i;
	
	for (i=0; i< MAX_PRIORITY; i++)
	{
		INIT_LIST_HEAD(&all_task[i]);
	}
}
