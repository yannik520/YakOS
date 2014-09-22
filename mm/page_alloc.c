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
#include <kernel/list.h>
#include <kernel/printk.h>
#include <arch/mmu.h>
#include <arch/arch.h>
#include <mm/page_alloc.h>

#define MIN(x, y)	((x) <= (y) ? (x) : (y))
#define MAX(x, y)	((x) <= (y) ? (y) : (x))

static struct page	*memmap_pages;
struct zone		 zones[MAX_NR_ZONES];
char *zones_name[MAX_NR_ZONES] = {"normal"};

void print_free_list(void) {
	struct zone		*zone  = &zones[ZONE_NORMAL];
	struct free_area	*area;
	struct page		*page;
	int			 order = MAX_ORDER - 1;
	int			 i;

	while (order >= 0) {
		area = &zone->free_area[order];
		list_for_each_entry(page, &area->free_list, list) {
			for (i = order; i< MAX_ORDER - 1; i++) {
				printk("    ");
			}

			printk("%d\n", page->index);
		}
		order--;
	}
}

void insert_to_free_area(struct zone *zone, struct page *page, uint32_t page_num) {
	int		 order	  = MAX_ORDER - 1;
	struct page	*cur_page = page;
	uint32_t	 cur_num  = page_num;

	do {
		uint32_t element_num = 1 << order;
		if (cur_num >= element_num) {
			INIT_LIST_HEAD((struct list_head *)&cur_page->list);
			list_add_tail(&cur_page->list, &zone->free_area[order].free_list);
			zone->free_area[order].nr_free++;
			cur_page += element_num;
			cur_num	 -= element_num;
		}
		else {
			if (order > 0) {
				order--;
			}
		}
	} while((int32_t)cur_num > 0);
}

void page_alloc_init(uint32_t *addr, uint32_t size) {
	struct zone	*zone;
	unsigned int	 i;
	uint32_t	 used_pages;
	struct page	*cur_page;
	uint32_t	*addr_origin = addr;
	
	
	if (NULL == addr) {
		printk("%s %d: addr is null!\n", __FUNCTION__, __LINE__);
		return;
	}

	/* Align to page size */
	addr = (uint32_t *)(((uint32_t)addr + PAGE_SIZE - 1) & PAGE_MASK);
	size = (size - ((uint32_t)addr - (uint32_t)addr_origin)) & PAGE_MASK;

	/* Just to initialize normal zone */
	zone		     = &zones[ZONE_NORMAL];
	zone->name	     = zones_name[ZONE_NORMAL];
	zone->zone_start_pfn = (unsigned long)addr >> PAGE_SHIFT;
	zone->spanned_pages  = size >> PAGE_SHIFT;

	for (i = 0; i < MAX_ORDER; i++) {
		INIT_LIST_HEAD(&zone->free_area[i]);
	}
	used_pages	    = ((zone->spanned_pages * sizeof(struct page)) >> PAGE_SHIFT) + 1;
	zone->managed_pages = zone->spanned_pages - used_pages;

	sema_init(&zone->lock, 1);
	
	memmap_pages = (struct page *)addr;
	
	for (i = 0; i < zone->managed_pages; i++) {
		cur_page	= memmap_pages + i;
		cur_page->index = i;
	}
	insert_to_free_area(zone, memmap_pages+used_pages, zone->managed_pages);

	/* print_free_list(); */
}

static inline
int get_order(unsigned long size)
{
	int order;

	size--;
	size >>= PAGE_SHIFT;
	order = fls(size);

	return order;
}

static inline void set_page_order(struct page *page, int order)
{
	page->private = order;
	__SetPageBuddy(page);
}

static inline void rmv_page_order(struct page *page)
{
	page->private = 0;
	__ClearPageBuddy(page);
}

static inline void expand(struct zone *zone, struct page *page,
	int low, int high, struct free_area *area)
{
	unsigned long size = 1 << high;

	while (high > low) {
		area--;
		high--;
		size >>= 1;

		list_add(&page[size].list, &area->free_list);
		area->nr_free++;
		set_page_order(&page[size], high);
	}
}

static struct page *__rmqueue(struct zone *zone, unsigned int order) {
	unsigned int		 current_order;
	struct free_area	*area;
	struct page		*page;

	for (current_order = order; current_order < MAX_ORDER; ++current_order) {
		area = &(zone->free_area[current_order]);
		if (list_empty(&area->free_list)) {
			continue;
		}
		
		page = list_entry(area->free_list.next, struct page, list);
		list_del(&page->list);
		rmv_page_order(page);
		area->nr_free--;
		expand(zone, page, order, current_order, area);
		page->private = order;
		return page;
	}

	return NULL;
}

void *page_address(struct page *page) {
	unsigned long page_idx;

	page_idx = page - memmap_pages;

	return (void *)((unsigned long)memmap_pages + (page_idx << PAGE_SHIFT));
}

void *alloc_pages(size_t size) {

	unsigned int	 order = get_order(size);
	struct zone	*zone  = &zones[ZONE_NORMAL];
	struct page	*page;
	unsigned long	 used_pages;
	
	if (order >= MAX_ORDER) {
		return NULL;
	}

	down(&zone->lock);
	page = __rmqueue(zone, order);
	up(&zone->lock);

	/* print_free_list(); */
	return page_address(page);
}

static inline unsigned long
__find_buddy_index(unsigned long page_idx, unsigned int order)
{
	return page_idx ^ (1 << order);
}

static inline int page_is_buddy(struct page *page, struct page *buddy, unsigned int order)
{
	if (PageBuddy(buddy) && page_order(buddy) == order) {
		return 1;
	}
	return 0;
}


static inline void free_one_page(struct zone *zone,
				 struct page *page,
				 unsigned int order) {
	unsigned long	 page_idx;
	unsigned long	 buddy_idx;
	unsigned long	 combined_idx;
	struct page	*buddy;

	page_idx = page->index;

	while (order < MAX_ORDER-1) {
		buddy_idx = __find_buddy_index(page_idx, order);
		buddy	  = page + (buddy_idx - page_idx);
		if (!page_is_buddy(page, buddy, order))
			break;

		list_del(&buddy->list);
		zone->free_area[order].nr_free--;
		rmv_page_order(buddy);
		
		combined_idx = buddy_idx &	page_idx;
		page	     = page + (combined_idx - page_idx);
		page_idx     = combined_idx;
		order++;
	}
	set_page_order(page, order);
	list_add(&page->list, &zone->free_area[order].free_list);
	zone->free_area[order].nr_free++;
}

void __free_pages(struct page *page, unsigned int order) {
	struct zone *zone = &zones[ZONE_NORMAL];

	down(&zone->lock);
	free_one_page(zone, page, order);
	up(&zone->lock);
}

struct page *virt_to_page(void *addr) {
	unsigned int pfn;
	struct zone *zone;
	unsigned int index;

	zone = &zones[ZONE_NORMAL];
	pfn  = (unsigned long)addr >> PAGE_SHIFT;
	
	if ((pfn > (zone->zone_start_pfn + zone->spanned_pages)) ||
	    (pfn < zone->zone_start_pfn)) {
		return NULL;
	}

	index = pfn - zone->zone_start_pfn;
	return (memmap_pages + index);
}

void free_pages(void *addr) {
	struct page *page;
	unsigned int used_pages;
	unsigned int order;

	if (0 == addr) {
		return;
	}

	page = virt_to_page(addr);
	if (NULL != page) {
		order = page->private;
		__free_pages(page, order);
		/* print_free_list(); */
	}
}
