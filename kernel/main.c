#include "kernel/task.h"
#include "kernel/printf.h"
#include "kernel/malloc.h"
#include "arch/platform.h"

#define TASK_STACK_SIZE	0x800 //2k

extern int		 __heap;
extern unsigned int	 stack_top;
extern struct list_head	*all_task[MAX_PRIORITY];
extern task_t		*current_task;


task_t init;

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


void c_entry() {
  
	struct task_t *task1;

	platform_init();

	printf("start ...!\n");

	init.sp = &stack_top;
	init.priority = MAX_PRIORITY-1;
	INIT_LIST_HEAD(&init.list);
	current_task = &init;

	malloc_init(&__heap, 0x2000);

	task_init();
	task1 = task_create(func1, 0, 0, TASK_STACK_SIZE);
	task_create(func2, 0, 0, TASK_STACK_SIZE);

	printf("start schedule\n");
	task_schedule();
	//arch_context_switch(&init, task1);
}
