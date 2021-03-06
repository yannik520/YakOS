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

#include <string.h>
#include <kernel/printk.h>
#include <kernel/types.h>
#include <kernel/task.h>
#include <arch/text.h>
#include <init.h>
#include <module/module.h>
#include <fs/vfsfs.h>
#include <fs/vfsfat.h>
#include <kernel/workqueue.h>

char console_buffer[CONSOLE_BUFFER_SIZE];
static char erase_seq[] = "\b \b";    /* erase sequence	*/
static char   tab_seq[] = "        "; /* used to expand TABs	*/
LIST_HEAD(commands);



static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0) {
		return (p);
	}

	if (*(--p) == '\t') {			/* will retype the whole line	*/
		while (*colp > plen) {
			printk("%s", erase_seq);
			(*colp)--;
		}
		for (s=buffer; s<p; ++s) {
			if (*s == '\t') {
				printk("%s", tab_seq+((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				printk("%c", *s);
			}
		}
	} else {
		printk("%s", erase_seq);
		(*colp)--;
	}
	(*np)--;
	return (p);
}

int readline(const char *const prompt)
{
	char	*p     = console_buffer;
	char	*p_buf = p;
	int	 plen  = 0;
	int	 n     = 0;	/* buffer index		*/
	int	 col;
	char	 c;


	if (prompt) {
		plen = strlen(prompt);
		printk("%s", prompt);
	}
	col = plen;

	for (;;) {
		c = getchar();
		
		switch (c) {
		case '\r':	/* Enter		*/
		case '\n':
			*p = '\0';
			printk ("\r\n");
			return (p - p_buf);

		case '\0':	/* nul			*/
			continue;

		case 0x03:				/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			return (-1);

		case 0x15:	/* ^U - erase line	*/
			while (col > plen) {
				printk("%s", erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		case 0x17:/* ^W - erase word	*/
			p=delete_char(p_buf, p, &col, &n, plen);
			while ((n > 0) && (*p != ' ')) {
				p=delete_char(p_buf, p, &col, &n, plen);
			}
			continue;

		case 0x08:				/* ^H  - backspace	*/
		case 0x7F:				/* DEL - backspace	*/
			p=delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < CONSOLE_BUFFER_SIZE-2) {
				if (c == '\t') {	/* expand TABs		*/
					printk("%s", tab_seq+(col&07));
					col += 8 - (col&07);
				} else {
					++col;		/* echo input		*/
					printk("%c", c);
				}
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				printk ("%c",'\a');
			}
		}
	}
}

#define TOLOWER(x) ((x) | 0x20)
#define isxdigit(c)	(('0' <= (c) && (c) <= '9') \
			 || ('a' <= (c) && (c) <= 'f') \
			 || ('A' <= (c) && (c) <= 'F'))

#define isdigit(c)	('0' <= (c) && (c) <= '9')
#define islower(c)	('a' <= (c) && (c) <= 'z')
#define isupper(c)	('A' <= (c) && (c) <= 'Z')

// convert string to upper
void simple_str2up(char *str)
{
	if (NULL == str) {
		return;
	}
	
	while (*str) {
		if (islower(*str))
		{
			*str -= 'a'-'A';
		}
		str++;
	}
}

// convert string to lower
void simple_str2lo(char *str)
{
	if (NULL == str) {
		return;
	}
	
	while (*str) {
		if (isupper(*str))
		{
			*str -= 'A'-'a';
		}
		str++;
	}
}

static unsigned int simple_guess_base(const char *cp)
{
	if (cp[0] == '0') {
		if (TOLOWER(cp[1]) == 'x' && isxdigit(cp[2]))
			return 16;
		else
			return 8;
	} else {
		return 10;
	}
}

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = 0;

	if (!base)
		base = simple_guess_base(cp);

	if (base == 16 && cp[0] == '0' && TOLOWER(cp[1]) == 'x')
		cp += 2;

	while (isxdigit(*cp)) {
		unsigned int value;

		value = isdigit(*cp) ? *cp - '0' : TOLOWER(*cp) - 'a' + 10;
		if (value >= base)
			break;
		result = result * base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;

	return result;
}

void help(void) {
	struct shell_command	*cur_cmd;
	int i;

	printk("All Supported Commands:\n");
	for (i=0; i<80; i++)
		printk("-");
	printk("\n");
	list_for_each_entry(cur_cmd, &commands, list) {
		printk("--> %s:\n", cur_cmd->command);
		printk("\t%s.\n", cur_cmd->description);
	}
	for (i=0; i<80; i++)
		printk("-");
	printk("\n\n");
}


extern unsigned int stack_top;

CMD_FUNC(insmod) {
	struct k_module	*mod;
	unsigned char	*file_buf;
	unsigned char	*buf;
	size_t		 count;
	size_t		 size	  = PAGE_SIZE;
	int		 fd;
	struct vfs_node	 file;
	char		*path	  = args;
	char new_path[128]	  = {'\0'};
	char		*cur_path;
	void (*pFun)();

	if (NULL == path) {
		printk("Please give a correct path!\n");
		return -1;
	}

	cur_path = vfs_get_cur_path();
	strcpy(new_path, cur_path);
	if ((NULL != path) && (0 < strlen(path))) {
		vfs_change_path(new_path, path);
	}

	simple_str2up(new_path);

	mod = alloc_kmodule();
	if (unlikely(NULL == mod))
	{
		printk("kmodule alloc failed!\n");
		return -1;
	}

	file_buf = kmalloc(size);
	if (unlikely(NULL == file_buf))
	{
		printk("file buffer alloc failed!\n");
		return -1;
	}
	buf = file_buf;
	
	if ((fd = vfs_open(new_path, &file)) == -1) {
		printk("vfs_open failed\n");
		return -1;
	}

	while (vfs_read(fd, buf, size, &count) != -1 && (count != 0)) {
		if (count > 0) {
			buf += count;
			size = PAGE_SIZE - count;
			//printk("%s\n", buf);
		}
	}

	if (vfs_close(fd)) {
		printk("vfs_close failed\n");
		return -1;
	}

	load_kmodule((unsigned int)file_buf, mod);
	kfree(file_buf);
	/* load_kmodule(0xc0100000, mod); */
	/* printk("module name: %s numb_syms=%d\n", ((struct module *)elfloader_autostart_processes)->name, */
	/* 	       ((struct module *)elfloader_autostart_processes)->num_syms); */
	pFun = mod->entry->syms[0].value;
	/* printk("code: %x %x %x\n", */
	/*        *((unsigned int *)elfloader_autostart_processes), */
	/*        *((unsigned int *)(elfloader_autostart_processes) + 1), */
	/*        *((unsigned int *)(elfloader_autostart_processes) + 2)); */
	(*pFun)();
	
	return 0;
}

CMD_FUNC(rmmod) {
	struct k_module	*mod;
	char		*mod_name = args;
	void (*pFun)();

	if (NULL == mod_name) {
		printk("Please give a correct module name!\n");
		return -1;
	}

	mod = find_module_by_name(mod_name);
	if (NULL == mod)
	{
		printk("Cann't find the module!\n");
		return -1;
	}

	pFun = mod->entry->syms[1].value;
	(*pFun)();
	
	free_kmodule(mod);

	return 0;
}

CMD_FUNC(mount) {
	unsigned long fimage_addr;

	if (NULL == args) {
		return -1;
	}

	fimage_addr = simple_strtoul(args, &args, 16);

	if (fimage_addr == 0) {
		printk("Please give a correct address!");
		return -1;
	}

	if (vfs_mount((const uint8_t *)fimage_addr, "FAT")) {
		printk("FAT FS Mount Failed!\n");
		return -1;
	}
	return 0;
}

CMD_FUNC(ls) {
	size_t		 count;
	int		 fd;
	struct vfs_node	 file;
	char		*path = args;
	char new_path[128]    = {'\0'};
	char *cur_path = vfs_get_cur_path();
	char *buf;

	buf = kmalloc(1024);
	if (NULL == buf) {
		return -1;
	}
	
	strcpy(new_path, cur_path);
	if ((NULL != path) && (0 < strlen(path))) {
		vfs_change_path(new_path, path);
	}
	simple_str2up(new_path);
	
	if ((fd = vfs_open(new_path, &file)) == -1) {
		printk("vfs_open failed\n");
		return -1;
	}

	while (vfs_read(fd, buf, 1024, &count) != -1 && (count != 0)) {
		if (count > 0)
		{
			if (0 < strlen(buf)) {
				simple_str2lo(buf);
				printk("%s\n", buf);
			}
			memset(buf, 0, 1024);
		}
	}

	if (vfs_close(fd)) {
		printk("vfs_close failed\n");
		return -1;
	}
	
	kfree(buf);
	return 0;
}

CMD_FUNC(cd) {
	char	*cur_path = vfs_get_cur_path();
	char	*path	  = args;

	if (NULL == path)
		return -1;

	simple_str2lo(path);
	vfs_change_path(cur_path, path);
	
	return 0;
}

CMD_FUNC(kmsg) {
	kmsg_dump();
	return 0;
}

CMD_FUNC(help) {
	help();
	return 0;
}

SHELL_COMMAND(kmsg_command, "kmsg", "help: shows all kernel message", CMD_FUNC_NAME(kmsg));
SHELL_COMMAND(insmod_command, "insmod", "help: insmod the given module", CMD_FUNC_NAME(insmod));
SHELL_COMMAND(rmmod_command, "rmmod", "help: remove the given module", CMD_FUNC_NAME(rmmod));
SHELL_COMMAND(mount_command, "mount", "help: mount the given file system", CMD_FUNC_NAME(mount));
SHELL_COMMAND(ls_command, "ls", "help: list all file or directory", CMD_FUNC_NAME(ls));
SHELL_COMMAND(cd_command, "cd", "help: change to the specified directory", CMD_FUNC_NAME(cd));
SHELL_COMMAND(help_command, "help", "help: display all commands", CMD_FUNC_NAME(help));

void shell_unregister_command(struct shell_command *cmd)
{
	list_del(&cmd->list);
}

void shell_register_command(struct shell_command *cmd)
{
	struct shell_command *pos;
	
	if (NULL == cmd) {
		return;
	}

	if (list_empty(&commands)) {
		list_add_tail(&cmd->list, &commands);
	}
	else {
		list_for_each_entry(pos, &commands, list) {
			if (strcmp(pos->command, cmd->command) > 0) {
				list_add_tail(&cmd->list, &pos->list);
				return;
			}
		}
		list_add_tail(&cmd->list, &commands);
	}
}

int run_command(const char *cmd)
{
	char			 cmdbuf[CONSOLE_BUFFER_SIZE];
	char			*str = cmdbuf;
	char			*args;
	struct shell_command	*cur_cmd;

	if (!cmd || !*cmd) {
		return -1;
	}

	while(*cmd == ' ') {
		cmd++;
	}

	strcpy(cmdbuf, cmd);

	list_for_each_entry(cur_cmd, &commands, list) {
		if (0 == strncmp(cur_cmd->command, str, strlen(cur_cmd->command))) {
			args = strchr(str, ' ');
			if (args != NULL) {
				args++;
				while(*args == ' ') {
					args++;
				}
			}
	
			if ((NULL != args) &&
			    ((0 == strcmp("--help", args)) ||
			     (0 == strcmp("-h", args)))) {
				printk("%s\n", cur_cmd->description);
			}
			else {
				cur_cmd->func(args);
			}
			return 0;
		}
	}

	printk("unknown command\n");
	return 0;
}

int init_shell(void *arg)
{
	int len;
	static char lastcommand[CONSOLE_BUFFER_SIZE] = { 0, };

	shell_register_command(&kmsg_command);
	shell_register_command(&insmod_command);
	shell_register_command(&rmmod_command);
	shell_register_command(&mount_command);
	shell_register_command(&ls_command);
	shell_register_command(&cd_command);
	shell_register_command(&help_command);

	for (;;) {
		printk("%s # ", vfs_get_cur_path());
		len = readline("");
		
		if (len > 0) {
			strcpy(lastcommand, console_buffer);
			//printk("%s\n", lastcommand);
			run_command(lastcommand);
		}
	}

	return 0;
}
