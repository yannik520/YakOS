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
	task1 = task_alloc("task1", 0x800, 1);
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
	task2 = task_alloc("task2", 0x800, 1);
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
	task3 = task_alloc("task3", 0x800, 1);
	if (NULL == task3)
	{
		return;
	}
	ret = task_create(task3, func3, 0);
	if (ret)
	{
		printf("task create error.\n");
	}

	while(1)
	{
		printf("main\n");
	}

	task_free(task1);
	task_free(task2);
	task_free(task3);
}
