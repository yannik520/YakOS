#include "kernel/task.h"
#include "kernel/printf.h"
#include "kernel/malloc.h"
#include "arch/platform.h"
#include "kernel/type.h"

#define TASK_STACK_SIZE		0x800    //2k
#define HEAP_SIZE		0x100000 //1M

extern int			__heap;
extern unsigned int		stack_top;
extern struct list_head		*all_task[MAX_PRIORITY];
extern task_t			*current_task;

task_t *init_task_create()
{
	unsigned int	*stack_addr;
	task_t		*init;

	init = (task_t *)kmalloc(sizeof(task_t));
	if (init == (task_t *)0)
	{
		printf("alloc task_t error!\n");
		return NULL;
	}

	stack_addr = (unsigned int *)kmalloc(TASK_STACK_SIZE);
	if (stack_addr == NULL)
	{
		kfree(init);
		return NULL;
	}

	init->sp = (unsigned int)stack_addr + TASK_STACK_SIZE;
	init->priority = MAX_PRIORITY-1;
	INIT_LIST_HEAD(&init->list);
	current_task = init;
}

void func1(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: func1 running\n", i);
	}
}

void func2(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: func2 running\n", i);
	}
}

void func3(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: func3 running\n", i);
	}
}


void c_entry()
{
  
	struct task_t *task;

	printf("start ...!\n");

	platform_init();

	kmalloc_init(&__heap, HEAP_SIZE);

	task_init();

	init_task_create();

	task = task_create(func1, 0, 0, TASK_STACK_SIZE);
	if (task == NULL)
	{
		printf("task create error.\n");
	}
	task = task_create(func2, 0, 0, TASK_STACK_SIZE);
	if (task == NULL)
	{
		printf("task create error.\n");
	}
	task = task_create(func3, 0, 0, TASK_STACK_SIZE);
	if (task == NULL)
	{
		printf("task create error.\n");
	}

	printf("start schedule\n");
	task_schedule();
}
