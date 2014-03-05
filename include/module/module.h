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
#ifndef __MODULE_H__
#define __MODULE_H__

#include <kernel/list.h>
#include <module/symbols.h>
#include <module/elf.h>

#define MODULE_NAME_LEN (64 - sizeof(unsigned long))

struct module_entry {
  char name[MODULE_NAME_LEN];
  unsigned int num_syms;
  struct symbols syms[];
};

struct segment_info {
	struct relevant_section text;
	struct relevant_section rodata;
  	struct relevant_section data;
	struct relevant_section bss;
};

struct k_module {
	struct list_head list;
	struct segment_info seg_info;
	struct module_entry *entry;
	int (*init_module)();
	void (*exit_module)();
};

#endif
