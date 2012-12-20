#include "kernel/task.h"
#include "kernel/printf.h"
#include "kernel/malloc.h"
#include "arch/arch_task.h"
#include "arch/interrupts.h"
#include "arch/timer.h"

extern int		 __heap;
extern unsigned int	 stack_top;
extern struct list_head	*all_task[MAX_TASKS];
extern task_t *current_task;

task_t init;

static enum handler_return timer_tick(void *arg, time_t now)
{
  printf("timer_tick\n");
  return INT_RESCHEDULE;
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


void c_entry() {
  
	struct task_t *task1;
	console_init();
	printf("start ...!\n");

	platform_init_interrupts();
	platform_init_timer();
	platform_set_periodic_timer(timer_tick, 0, 1000); /* 10ms */

	init.sp = &stack_top;
	init.priority = MAX_TASKS-1;
	INIT_LIST_HEAD(&init.list);
	current_task = &init;

	malloc_init(&__heap, 0x2000);
	task_init();
	task1 = task_create(func1, 0, 0, 0x800);
	task_create(func2, 0, 0, 0x800);
	printf("start schedule\n");
	task_schedule();
	//arch_context_switch(&init, task1);

}
