#include "kernel/task.h"
#include "kernel/printf.h"
#include "kernel/malloc.h"
#include "kernel/type.h"
#include "arch/arch_task.h"

extern unsigned int stack_top;

task_t			*current_task;
struct list_head	 all_task[MAX_PRIORITY];
unsigned int		 task_bitmap;

void initial_task_func(void)
{
	int ret;
	
	printf("initial_task_func: entry=0x%x\n", current_task->entry);
	exit_critical_section();
	ret = current_task->entry(current_task->args);
}

task_t *task_create(task_routine entry, void *args, unsigned int priority, int stack_size)
{
	task_t *t;
	unsigned int *stack_addr;

	t = (task_t *)kmalloc(sizeof(task_t));
	if (t == (task_t *)0)
	{
		printf("alloc task_t error!\n");
		return NULL;
	}

	stack_addr = (unsigned int *)kmalloc(stack_size);
	if (stack_addr == NULL)
	{
		kfree(t);
		return NULL;
	}

	t->stack = (unsigned int *)((unsigned int)stack_addr + stack_size);
	t->stack_size = stack_size;
	t->entry = entry;
	t->args = args;
	t->priority = priority;

	arch_task_initialize(t);
	INIT_LIST_HEAD(&t->list);
	list_add_tail(&t->list, &all_task[priority]);

	task_bitmap |= 1 << priority;

	return t;
}

void task_schedule(void)
{
	struct list_head *list = &current_task->list;
	task_t *old_task = current_task;
	task_t *new_task;
	unsigned int i;

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

	current_task = new_task;

	arch_context_switch(old_task, new_task);
out:
	printf("task_schedule end!\n");
}

void task_init(void)
{
	int i;
	
	for (i=0; i< MAX_PRIORITY; i++)
	{
		INIT_LIST_HEAD(&all_task[i]);
	}
}
