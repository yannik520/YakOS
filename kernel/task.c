#include "kernel/task.h"
#include "kernel/printf.h"
#include "kernel/malloc.h"
#include "kernel/type.h"
#include "arch/arch_task.h"

extern unsigned int stack_top;

task_t *current_task;
struct list_head all_task[MAX_TASKS];
unsigned int task_bitmap;
unsigned int task_index = 0;

void initial_task_func(void)
{
	int ret;
	
	printf("initial_task_func: entry=0x%x\n", current_task->entry);
	ret = current_task->entry(current_task->args);
}

task_t *task_create(task_routine entry, void *args, unsigned int priority, int stack_size)
{
	task_t *t;
	unsigned int stack_addr = (int)(&stack_top - (task_index + 1) * stack_size);
	printf("task_create, task_index=0x%x, stack_top=0x%x,stack_size=0x%x stack_addr=0x%x\n", task_index, &stack_top, stack_size, stack_addr);

	t = (task_t *)malloc(sizeof(task_t));
	if (t == (task_t *)0)
	{
		printf("alloc task_t error!\n");
		return 0;
	}
	printf("task_add=0x%x, sizeof_task=0x%x\n", (unsigned int)t, sizeof(task_t));
	t->stack = (unsigned int *)stack_addr;
	t->stack_size = stack_size;
	t->entry = entry;
	t->args = args;
	t->priority = priority;
	printf("before in arch_task_initialize!\n");
	arch_task_initialize(t);
	INIT_LIST_HEAD(&t->list);
	list_add_tail(&t->list, &all_task[priority]);
	printf("after list_add_tail!\n");
	task_bitmap |= 1 << priority;
	task_index++;
	printf("task_create ok\n");
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
		printf("start switch priority group!\n");

		for (i=0; i<MAX_TASKS; i++)
		{
			if ((task_bitmap >> i) & 1)
			{
				break;
			}
		}
		
		printf("current_priority=%d\n", i);
		if (i < MAX_TASKS)
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
		printf("Needn't switch priority group!\n");
		new_task = (task_t *)current_task->list.next;
	}

	current_task = new_task;
	printf("old_task->stack=0x%x, new_task->stack=0x%x\n",
	       (unsigned int)old_task->stack, (unsigned int)new_task->stack);
	arch_context_switch(old_task, new_task);
out:
	printf("No other task in run queue now!\n");
}

void task_init(void)
{
	int i;
	
	for (i=0; i< MAX_TASKS; i++)
	{
		INIT_LIST_HEAD(&all_task[i]);
	}
}
