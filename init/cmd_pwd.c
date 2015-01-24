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
#include <kernel/types.h>
#include <kernel/printk.h>
#include <module/module.h>
#include <fs/vfsfs.h>
#include <fs/vfsfat.h>
#include <init.h>

CMD_FUNC(pwd) {
	char *path = vfs_get_cur_path();

	printk("%s\n", path);

	return 0;
}

SHELL_COMMAND(pwd_command, "pwd", "help: To print working directory", CMD_FUNC_NAME(pwd));

int init_module (void)
{
	printk("start init pwd command!\n");

	shell_register_command(&pwd_command);
  
	return 0;
}

void exit_module(void)
{
	printk("delete pwd command!\n");
}

struct module_entry mod_entry = {
	.name = "pwd",
	.num_syms = 2,
	.syms = {{"init_module", &init_module},
		 {"exit_moduel", &exit_module}}
};
