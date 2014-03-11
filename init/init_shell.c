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
#include <kernel/type.h>
#include <kernel/task.h>
#include <arch/text.h>
#include <init.h>
#include <module/module.h>

char console_buffer[CONSOLE_BUFFER_SIZE];
static char erase_seq[] = "\b \b";    /* erase sequence	*/
static char   tab_seq[] = "        "; /* used to expand TABs	*/

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

void (*pFun)();
extern unsigned int stack_top;

int run_command(const char *cmd)
{
	char		 cmdbuf[CONSOLE_BUFFER_SIZE];
	char		*str = cmdbuf;
	struct kmodule	*mod;

	if (!cmd || !*cmd) {
		return -1;
	}
	
	strcpy(cmdbuf, cmd);

	if (0 == strcmp("kmsg", str)) {
		kmsg_dump();
	}
	else if (0 == strcmp("exec", str)) {
		mod = alloc_kmodule();
		if (NULL == mod)
		{
			printk("kmodule alloc failed!\n");
			return -1;
		}
		load_kmodule(0xc0100000, mod);
		/* printk("module name: %s numb_syms=%d\n", ((struct module *)elfloader_autostart_processes)->name, */
		/* 	       ((struct module *)elfloader_autostart_processes)->num_syms); */
		pFun = this_module->entry->syms[0].value;
		/* printk("code: %x %x %x\n", */
		/*        *((unsigned int *)elfloader_autostart_processes), */
		/*        *((unsigned int *)(elfloader_autostart_processes) + 1), */
		/*        *((unsigned int *)(elfloader_autostart_processes) + 2)); */
		(*pFun)();
	}
	else {
		printk("unknown command\n");
	}
		
}

int init_shell(void *arg)
{
	int len;
	static char lastcommand[CONSOLE_BUFFER_SIZE] = { 0, };

	for (;;) {
		len = readline("# ");
		
		if (len > 0) {
			strcpy(lastcommand, console_buffer);
			printk("%s\n", lastcommand);
			run_command(lastcommand);
		}
	}

	return 0;
}
