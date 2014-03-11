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
#ifndef __RAM_SEGMENTS_H__
#define __RAM_SEGMENTS_H__

#include <kernel/type.h>
#include <kernel/malloc.h>
#include <string.h>
#include <module/module.h>

struct ram_output
{
	struct module_output	 output;
	char			*base;
	unsigned int		 offset;
	void			*text;
	void			*rodata;
	void			*data;
	void			*bss;
};

static void * allocate_segment(struct module_output * const output,
			       unsigned int type, int size)
{
	struct ram_output * const	 ram   = (struct ram_output *)output;
	void				*block = kmalloc(size);

	if (!block) return NULL;

	switch(type) {
	case MODULE_SEG_TEXT:
		if (ram->text) kfree(ram->text);
		ram->text = block;
		break;
	case MODULE_SEG_RODATA:
		if (ram->rodata) kfree(ram->rodata);
		ram->rodata = block;
		break;
	case MODULE_SEG_DATA:
		if (ram->data) kfree(ram->data);
		ram->data = block;
		break;
	case MODULE_SEG_BSS:
		if (ram->bss) kfree(ram->bss);
		ram->bss = block;
		break;
	default:
		kfree(block);
		return NULL;
	}
	return block;
}

static int start_segment(struct module_output *output,
			 unsigned int type, void *addr, int size)
{
	((struct ram_output*)output)->base   = addr;
	((struct ram_output*)output)->offset = 0;

	return MODULE_OK;
}

static int end_segment(struct module_output *output)
{
	return MODULE_OK;
}

static int write_segment(struct module_output *output, const char *buf,
			 unsigned int len)
{
	struct ram_output * const ram = (struct ram_output *)output;

	memcpy(ram->base + ram->offset, buf, len);
	ram->offset += len;

	return len;
}

static unsigned int segment_offset(struct module_output *output)
{
	return ((struct ram_output*)output)->offset;
}

static const struct module_output_ops mod_output_ops =
{
	allocate_segment,
	start_segment,
	end_segment,
	write_segment,
	segment_offset
};


static struct ram_output seg_output = {
	{&mod_output_ops},
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL
};

struct module_output *mod_output = &seg_output.output;

#endif
