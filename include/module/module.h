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
#include <module/elf.h>
#include <module/symtab.h>

extern struct list_head	 k_module_root;
extern struct k_module	*this_module;
extern char		 module_unknown[30];

#define MODULE_OK                  0
#define MODULE_BAD_ELF_HEADER      1
#define MODULE_NO_SYMTAB           2
#define MODULE_NO_STRTAB           3
#define MODULE_NO_TEXT             4
#define MODULE_SYMBOL_NOT_FOUND    5
#define MODULE_SEGMENT_NOT_FOUND   6
#define MODULE_NO_STARTPOINT       7
#define MODULE_UNHANDLED_RELOC     8
#define MODULE_OUTOF_RANGE	   9
#define MODULE_RELOC_NOT_SORTED    10
#define MODULE_INPUT_ERROR	   11
#define MODULE_OUTPUT_ERROR	   12

#define MODULE_SEG_TEXT            1
#define MODULE_SEG_RODATA          2
#define MODULE_SEG_DATA            3
#define MODULE_SEG_BSS             4

#define MODULE_NAME_LEN (64 - sizeof(unsigned long))

struct module_entry {
	char		name[MODULE_NAME_LEN];
	unsigned int	num_syms;
	struct symbols	syms[];
};

struct segment_info {
	struct relevant_section text;
	struct relevant_section rodata;
  	struct relevant_section data;
	struct relevant_section bss;
};

struct k_module {
	struct list_head		 list;

	const struct module_output_ops	*ops;
	char				*base;
	unsigned int			 offset;

	struct segment_info		 seg_info;
	struct module_entry		*entry;
	int (*init_module)();
	void (*exit_module)();
};

/*
struct module_output {
	const struct module_output_ops *ops;
};
*/

#define module_output_alloc_segment(output, type, size)		\
	((output)->ops->allocate_segment(output, type, size))

#define module_output_start_segment(output, type, addr, size)		\
	((output)->ops->start_segment(output, type, addr, size))

#define module_output_end_segment(output)	\
	((output)->ops->end_segment(output))

#define module_output_write_segment(output, buf, len)		\
	((output)->ops->write_segment(output, buf, len))

#define module_output_segment_offset(output)	\
	((output)->ops->segment_offset(output))


struct module_output_ops {
	void * (*allocate_segment)(struct k_module *output,
				   unsigned int type, int size);
	int (*start_segment)(struct k_module *output,
			     unsigned int type, void *addr, int size);
	int (*end_segment)(struct k_module *output);
	int (*write_segment)(struct k_module *output, const char *buf,
			     unsigned int len);
	unsigned int (*segment_offset)(struct k_module *output);
};


struct k_module * alloc_kmodule(void);
void free_kmodule(struct k_module *kmod);
int load_kmodule(unsigned int input_addr,
		 struct k_module *mod);

#endif /* __MODULE_H__ */
