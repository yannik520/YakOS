#include "kernel/task.h"
#include "kernel/printf.h"
#include "kernel/malloc.h"
#include "arch/platform.h"
#include "kernel/type.h"

#define HEAP_SIZE		0x100000 //1M

extern unsigned int		__heap;

int func1(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: func1 running\n", i);
	}
	
	return 0;
}

int func2(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: func2 running\n", i);
	}

	return 0;
}

int func3(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: func3 running\n", i);
	}
	
	return 0;
}


void c_entry(void)
{
	task_t *task1;
	task_t *task2;
	task_t *task3;
	int ret;

	printf("start ...!\n");

	/*************** Init Platform ****************/
	platform_init();
	
	/*************** Init Task ****************/
	kmalloc_init(&__heap, HEAP_SIZE);
	task_init();
	task_create_init();
	
	/*************** Creating TASK1 ****************/
	task1 = (task_t *)kmalloc(sizeof(task_t));
	if (task1 == (task_t *)0)
	{
		printf("Alloc task_t error!\n");
		return;
	}
	memset(task1, 0, sizeof(task_t));
	memcpy(task1->name, "task1", strlen("task1"));
	task1->priority = 1;
	ret = task_create(task1, func1, 0);
	if (ret)
	{
		printf("task create error.\n");
	}

	/*************** Creating TASK2 ****************/
	task2 = (task_t *)kmalloc(sizeof(task_t));
	if (task2 == (task_t *)0)
	{
		printf("Alloc task_t error!\n");
		return;
	}
	memset(task2, 0, sizeof(task_t));
	memcpy(task2->name, "task2", strlen("task2"));
	task2->priority = 1;
	ret = task_create(task2, func2, 0);
	if (ret)
	{
		printf("task create error.\n");
	}

	/*************** Creating TASK3 ****************/
	task3 = (task_t *)kmalloc(sizeof(task_t));
	if (task3 == (task_t *)0)
	{
		printf("Alloc task_t error!\n");
		return;
	}
	memset(task3, 0, sizeof(task_t));
	memcpy(task3->name, "task3", strlen("task3"));
	task3->priority = 1;
	ret = task_create(task3, func3, 0);
	if (ret)
	{
		printf("task create error.\n");
	}

	while(1)
	{
		printf("main\n");
	}
}
