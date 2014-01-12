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
#include <kernel/task.h>
#include <kernel/printf.h>
#include <kernel/malloc.h>
#include <arch/platform.h>
#include <kernel/type.h>
#include <kernel/timer.h>
#include <kernel/semaphore.h>

#define HEAP_SIZE		0x100000 //1M

extern unsigned int	__heap;
int			sum = 0;
struct semaphore	sem;

int func1(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		if(sum == 0)
			down(&sem);
		sum++;
		printf("%d: tfunc1 running\n", i);
		task_sleep(150);
		if(sum == 30)
			up(&sem);

	}
	
	return 0;
}

int func2(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		printf("%d: tfunc2 running\n", i);
		if ( i == 100)
		{
			task_exit(0);
		}
		task_sleep(120);
	}

	return 0;
}

int func3(void *arg)
{
	int i;

	for (i=0;;i++)
	{
		down(&sem);
		printf("sum=%d\n", sum);
		up(&sem);
		printf("%d: tfunc3 running\n", i);
		task_sleep(200);

	}
	
	return 0;
}

void c_entry(void)
{
	task_t	*task1;
	task_t	*task2;
	task_t	*task3;
	int	 ret;

	printf("start ...!\n");

	/*************** Init Platform ****************/
	platform_init();
	timer_init();

	/*************** Init Task ****************/
	kmalloc_init(&__heap, HEAP_SIZE);
	task_init();
	task_create_init();
	
	/*************** Creating TASK1 ****************/
	task1 = task_alloc("task1", 0x2000, 1);
	if (NULL == task1)
	{
		return;
	}
	ret = task_create(task1, func1, 0);
	if (ret)
	{
		printf("task create error.\n");
	}

	/*************** Creating TASK2 ****************/
	task2 = task_alloc("task2", 0x2000, 1);
	if (NULL == task2)
	{
		return;
	}
	ret = task_create(task2, func2, 0);
	if (ret)
	{
		printf("task create error.\n");
	}

	/*************** Creating TASK3 ****************/
	task3 = task_alloc("task3", 0x2000, 1);
	if (NULL == task3)
	{
		return;
	}
	ret = task_create(task3, func3, 0);
	if (ret)
	{
		printf("task create error.\n");
	}

	sema_init(&sem, 1);

	arch_enable_ints();
	while(1)
	{
		;
	}

	task_free(task1);
	task_free(task2);
	task_free(task3);
}
