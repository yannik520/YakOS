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
#include <kernel/printk.h>
#include <kernel/malloc.h>
#include <arch/platform.h>
#include <kernel/type.h>
#include <kernel/timer.h>
#include <kernel/semaphore.h>
#include <init.h>

#define HEAP_SIZE		0x100000 //1M

extern unsigned int	__heap;
int			sum = 0;
struct semaphore	sem;

void show_logo(void)
{
	printk("\n\n");
	printk(" _                       _  __                    _ \n");
	printk("| |   _   _ _ __ __  __ | |/ /___ _ __ _ __   ___| |\n");
	printk("| |  | | | | '_ \\\\ \\/ / | ' // _ \\ '__| '_ \\ / _ \\ |\n");
	printk("| |__| |_| | | | |>  <  | . \\  __/ |  | | | |  __/ |\n");
	printk("|_____\\__, |_| |_/_/\\_\\ |_|\\_\\___|_|  |_| |_|\\___|_|\n");
	printk("      |___/                                         \n");
	printk("\n\n");
}


void c_entry(void)
{
	task_t	*task_shell;
	int	 ret;

	show_logo();

	/*************** Init Platform ****************/
	platform_init();
	timer_init();

	/*************** Init Task ****************/
	kmalloc_init(&__heap, HEAP_SIZE);
	task_init();
	task_create_init();
	
	/*************** Creating Shell TASK ****************/
	task_shell = task_alloc("shell", 0x2000, 1);
	if (NULL == task_shell)
	{
		return;
	}

	ret = task_create(task_shell, init_shell, 0);
	if (ret) {
		printk("Create init shell task failed\n");
	}

	sema_init(&sem, 1);

	arch_enable_ints();

	while(1)
	{
		;
	}

	task_free(task_shell);

}
