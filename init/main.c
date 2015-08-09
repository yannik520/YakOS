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
#include <mm/malloc.h>
#include <arch/arch.h>
#include <arch/platform.h>
#include <kernel/types.h>
#include <kernel/timer.h>
#include <kernel/semaphore.h>
#include <init.h>
#include <fs/vfsfs.h>
#include <fs/vfsfat.h>
#include <driver/device.h>
#include <compiler.h>

int			sum = 0;
struct semaphore	sem;

void show_logo(void) {
	printk("\n");
	printk("▄▄▄    ▄▄▄           ▄▄          ▄▄▄▄      ▄▄▄▄   \n");
	printk(" ██▄  ▄██            ██         ██▀▀██   ▄█▀▀▀▀█  \n");
	printk("  ██▄▄██    ▄█████▄  ██ ▄██▀   ██    ██  ██▄      \n");
	printk("   ▀██▀     ▀ ▄▄▄██  ██▄██     ██    ██   ▀████▄  \n");
	printk("    ██     ▄██▀▀▀██  ██▀██▄    ██    ██       ▀██ \n");
	printk("    ██     ██▄▄▄███  ██  ▀█▄    ██▄▄██   █▄▄▄▄▄█▀ \n");
	printk("    ▀▀      ▀▀▀▀ ▀▀  ▀▀   ▀▀▀    ▀▀▀▀     ▀▀▀▀▀   \n");
	printk("\n");
}

void kmain(void)
{
	task_t	*task_shell;
	int	 ret;

	/*************** Init Arch ****************/
	arch_early_init();

	show_logo();

	/*************** Init Platform ****************/
	platform_init();
	timer_init();
	buses_init();

	/*************** Init Task ****************/
	task_init();
	task_create_init();

	/*************** Init Workqueu ****************/
	init_workqueues();
	
	/*************** Init File System ****************/
	register_filesystem(&fat_fs);
	
	/*************** Creating Shell TASK ****************/
	task_shell = task_alloc("shell", 0x2000, 5);
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
		enter_critical_section();
		arch_idle();
		task_schedule();
		exit_critical_section();
	}

	task_free(task_shell);
}
