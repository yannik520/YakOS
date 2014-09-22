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

#ifndef __PAGE_ALLOC_H__
#define __PAGE_ALLOC_H__

#include <kernel/types.h>
#include <kernel/semaphore.h>

#define MIN_ORDER	0
#define MAX_ORDER	11
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

enum zone_type {
	ZONE_NORMAL,
	MAX_NR_ZONES
};

struct page
{
	struct list_head	 list;
	uint32_t		 index;
	int			_mapcount;
	unsigned long		private;  
};

struct free_area {
	struct list_head	free_list;
	unsigned long		nr_free;
};

struct zone {
  	/* zone_start_pfn == zone_start_paddr >> PAGE_SHIFT */
	unsigned long		zone_start_pfn;
	unsigned long		spanned_pages;
	unsigned long		managed_pages;
	struct free_area	free_area[MAX_ORDER];
	struct semaphore	lock;
	const char		*name;
};

#define page_private(page)		((page)->private)

static inline unsigned long page_order(struct page *page)
{
	/* PageBuddy() must be checked by the caller */
	return page_private(page);
}

#define PAGE_BUDDY_MAPCOUNT_VALUE (-128)

static inline int PageBuddy(struct page *page)
{
	return page->_mapcount == PAGE_BUDDY_MAPCOUNT_VALUE;
}

static inline void __SetPageBuddy(struct page *page)
{
	page->_mapcount = PAGE_BUDDY_MAPCOUNT_VALUE;
}

static inline void __ClearPageBuddy(struct page *page)
{
	page->_mapcount = -1;
}

void page_alloc_init(uint32_t *addr, uint32_t size);
void *alloc_pages(size_t size);
void free_pages(void *addr);
#endif
