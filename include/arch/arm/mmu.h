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
#ifndef __MMU_H__
#define __MMU_H__

#include <kernel/types.h>

typedef uint32_t	pte_t;
typedef uint32_t	pgd_t;

#define PAGE_SHIFT	12
#define PAGE_SIZE	(1UL<<PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define page_start(_v)          ((_v) &				PAGE_MASK)
#define page_offset(_v)         ((_v) & (~PAGE_MASK))
#define page_align(_v)          (((_v) + (~PAGE_MASK)) &	PAGE_MASK)

#define SECTION_SHIFT	20
#define SECTION_SIZE	(1UL<<SECTION_SHIFT)
#define SECTION_MASK	(~(SECTION_SIZE-1))


/* Page directory */
#define PGDIR_SHIFT	20
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

#define pgd_index(addr)	((addr) >> PGDIR_SHIFT)
#define pgd_offset(pgd, addr)	((pgd_t *)(((pgd_t *)pgd) + pgd_index(addr)))
#define pte_index(addr)	(((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

#define ALIGN(P, ALIGNBYTES)  ((void*)( ((unsigned long)P + ALIGNBYTES -1) & ~(ALIGNBYTES-1) ) )

void	arm_mmu_init(void);

#endif
