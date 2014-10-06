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
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <const.h>
#include <arch/memmap.h>
#include <kernel/types.h>

#define UL(x) _AC(x, UL)

/* Physical */
#define PHYS_OFFSET     MAINMEM_BASE
#define PHYS_SIZE	MEMBANK_SIZE

/* Virtual */
#define PAGE_OFFSET     0xc0000000
#define TEXT_OFFSET     0x00008000

#define REGISTER_VADDR  (REGISTER_BASE + PAGE_OFFSET)

#define ARCH_SLOB_MINALIGN 8

typedef enum {
	MAP_DESC_TYPE_SECTION,
	MAP_DESC_TYPE_PAGE
} MAP_DESC_TYPE;

struct map_desc {
	uint32_t paddr;
	uint32_t vaddr;
	uint32_t length;
	uint32_t attr;
	MAP_DESC_TYPE type;
};

static inline uint32_t  __virt_to_phys(uint32_t x)
{
	return (uint32_t)x - PAGE_OFFSET + PHYS_OFFSET;
}

static inline unsigned long __phys_to_virt(uint32_t x)
{
	return x - PHYS_OFFSET + PAGE_OFFSET;
}

static inline uint32_t virt_to_phys(const volatile void *x)
{
	return __virt_to_phys((uint32_t)(x));
}

static inline void *phys_to_virt(uint32_t x)
{
	return (void *)__phys_to_virt(x);
}

#endif
