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

	if (INVALID_PRIORITY == task->priority)
	{
		task->priority = DEFAULT_PRIORITY;
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
