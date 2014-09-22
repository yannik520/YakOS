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
#include <arch/mmu.h>
#include <kernel/malloc.h>
#include <kernel/list.h>
#include <kernel/types.h>
#include <kernel/printk.h>
#include <compiler.h>
#include <mm/page_alloc.h>

void kmalloc_init(uint32_t *addr, uint32_t size)
{
	if (NULL == addr) {
		printk("%s %d: addr is null!\n", __FUNCTION__, __LINE__);
		return;
	}

	page_alloc_init(addr, size);
}

void *kmalloc(uint32_t size)
{
	int			 i;
	uint32_t		*mem = NULL;

	if (size == 0)
		return NULL;
	
       mem = alloc_pages(size);
       return mem;
}

void kfree(void *addr)
{
	free_pages(addr);
}
