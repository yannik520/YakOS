#include "task.h"
#include "printf.h"
#include "malloc.h"

extern int		 __heap;
extern unsigned int	 stack_top;
extern task_t		*all_task[MAX_TASKS];
extern int		 task_index;
extern int		 running_index;

volatile unsigned int * const UART0DR = (unsigned int *)0x101f1000;

void __puts(const char *s) {

	while(*s != '\0') {

		*UART0DR = (unsigned int)(*s);
		s++;
	}

}

task_t current;

void func1(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: func1 running\n", i);
		if (i != 0 && i%7 == 0)
		{
			task_schedule();
	  
		}
	}
}

void func2(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: func2 running\n", i);
		if (i != 0 && i%5 == 0)
		{
			task_schedule();
		}
	}
}

void c_entry() {

	printf("start ...!\n");
	current.sp = &stack_top;
	
	malloc_init(&__heap, 0x1000);
	arch_task_initialize(func1, 0);
	arch_task_initialize(func2, 0);
	running_index = 0;
	arch_context_switch(&current, all_task[running_index]);

}
