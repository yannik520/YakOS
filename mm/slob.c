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
#include <mm/slob.h>

static LIST_HEAD(free_slob_small);
static LIST_HEAD(free_slob_medium);
static LIST_HEAD(free_slob_large);

static inline int PageSlobFree(const struct page *page)			\
			{ return test_bit(PG_slob_free, &page->flags); }


static inline void __SetPageSlobFree(struct page *page)			\
			{ set_bit(PG_slob_free, &page->flags); }

static inline void __ClearPageSlobFree(struct page *page)		\
			{ clear_bit(PG_slob_free, &page->flags); }

static inline int slob_page_free(struct page *sp)
{
	return PageSlobFree(sp);
}

static void set_slob_page_free(struct page *sp, struct list_head *list)
{
	list_add(&sp->list, list);
	__SetPageSlobFree(sp);
}

static inline void clear_slob_page_free(struct page *sp)
{
	list_del(&sp->list);
	__ClearPageSlobFree(sp);
}

DEFINE_SEMAPHORE(sem);

static void set_slob(slob_t *s, slobidx_t size, slob_t *next)
{
	slob_t *base = (slob_t *)((unsigned long)s & PAGE_MASK);
	slobidx_t offset = next - base;

	if (size > 1) {
		s[0].units = size;
		s[1].units = offset;
	} else
		s[0].units = -offset;
}

static slobidx_t slob_units(slob_t *s)
{
	if (s->units > 0)
		return s->units;
	return 1;
}

static slob_t *slob_next(slob_t *s)
{
	slob_t *base = (slob_t *)((unsigned long)s & PAGE_MASK);
	slobidx_t next;

	if (s[0].units < 0)
		next = -s[0].units;
	else
		next = s[1].units;
	return base+next;
}


static int slob_last(slob_t *s)
{
	return !((unsigned long)slob_next(s) & ~PAGE_MASK);
}

void *slob_new_pages(size_t size)
{
	struct page *page;

	page = alloc_pages(size);
	if (!page) {
		return NULL;
	}

	return page_address(page);
}

static void slob_free_pages(void *b)
{
	free_pages(b);
}

void *slob_page_alloc(struct page *sp, size_t size, int align) {
	slob_t *prev, *cur, *aligned = NULL;
	int delta = 0, units= SLOB_UNITS(size);

	for (prev = NULL, cur = sp->freelist; ; prev = cur, cur = slob_next(cur)) {
		slobidx_t avail = slob_units(cur);
		
		if (align) {
			aligned = (slob_t *)ALIGN((unsigned long)cur + align - 1, align);
			delta = aligned - cur;
		}
		if (avail >= units + delta) { /* room enough? */
			slob_t *next;
			
			if (delta) { /* need to fragment head to align? */
				next = slob_next(cur);
				set_slob(aligned, avail - delta, next);
				set_slob(cur, delta, aligned);
				prev = cur;
				cur = aligned;
				avail = slob_units(cur);
			}
			
			next = slob_next(cur);
			if (avail == units) { /* exact fit? unlink. */
				if (prev) {
					set_slob(prev, slob_units(prev), next); /* unlink the current block */
				}
				else {
					sp->freelist = next;
				}
			}
			else { /* fragment */
				if (prev) {
					set_slob(prev, slob_units(prev), cur + units);
				}
				else {
					sp->freelist = cur + units;
				}
				set_slob(cur + units, avail - units, next);
			}

			sp->units -= units;
			if (!sp->units) {
				clear_slob_page_free(sp);
			}
			
			set_slob(cur, units, cur + units); /* just to store the size */

			return (void *)((unsigned int)cur + align);
		}
		if (slob_last(cur)) {
			return NULL;
		}
	}
}

void *slob_alloc(size_t size, int align) {
	struct page *sp;
	struct list_head *prev;
	struct list_head *slob_list;
	slob_t *b = NULL;

	if (size < SLOB_BREAK1)
		slob_list = &free_slob_small;
	else if (size < SLOB_BREAK2)
		slob_list = &free_slob_medium;
	else
		slob_list = &free_slob_large;

	down(&sem);

	list_for_each_entry(sp, slob_list, list) {
		if (sp->units < SLOB_UNITS(size))
			continue;

		/* Attempt to alloc */
		prev = sp->list.prev;
		b = slob_page_alloc(sp, size, align);
		if (!b)
			continue;
		
		/* Improve fragment distribution and reduce our average
		 * search time by starting our next search here. */
		if (prev != slob_list->prev &&
			slob_list->next != prev->next)
			list_move_tail(slob_list, prev->next);
		break;
	}
	
	up(&sem);

	if (!b) {
		b = slob_new_pages(PAGE_SIZE-1);
		if (!b)
			return NULL;
		sp = virt_to_page(b);
		
		down(&sem);
		sp->units = SLOB_UNITS(PAGE_SIZE);
		sp->freelist = b;
		INIT_LIST_HEAD(&sp->list);
		set_slob(b, SLOB_UNITS(PAGE_SIZE), b + SLOB_UNITS(PAGE_SIZE));
		set_slob_page_free(sp, slob_list);
		b = slob_page_alloc(sp, size, align);

		up(&sem);
	}
	
	return b;
}

void slob_free(void *block, int size) {
	struct page *sp;
	slob_t *prev, *next, *b = (slob_t *)block;
	slobidx_t units;
	struct list_head *slob_list;
	
	sp = virt_to_page(block);
	units = SLOB_UNITS(size);
	
	down(&sem);

	if (sp->units + units == SLOB_UNITS(PAGE_SIZE)) {
		/* Go directly to page allocator. Do not pass slob allocator */
		if (slob_page_free(sp))
			clear_slob_page_free(sp);
		up(&sem);
		sp->_mapcount = -1;
		slob_free_pages(b);
		return;
	}

	if (!slob_page_free(sp)) {
		/* This slob page is about to become partially free. */
		sp->units = units;
		sp->freelist = b;
		set_slob(b, units,
			 (void *)((unsigned long)(b + 
						  SLOB_UNITS(PAGE_SIZE)) & PAGE_MASK));
		if (size < SLOB_BREAK1)
			slob_list = &free_slob_small;
		else if (size < SLOB_BREAK2)
			slob_list = &free_slob_medium;
		else
			slob_list = &free_slob_large;
		set_slob_page_free(sp, slob_list);
		goto out;
	}

	/*
	 * Otherwise the page is already partially free, so find reinsertion
	 * point.
	 */
	sp->units += units;
	
	if (b < (slob_t *)sp->freelist) {
		if (b + units == sp->freelist) {
			units += slob_units(sp->freelist);
			sp->freelist = slob_next(sp->freelist);
		}
		set_slob(b, units, sp->freelist);
		sp->freelist = b;
	} else {
		prev = sp->freelist;
		next= slob_next(prev);
		while (b > next) {
			prev = next;
			next = slob_next(prev);
		}

		if (!slob_last(prev) && ((b + units) == next)) {
			/* merge with the following block */
			units += slob_units(next);
			set_slob(b, units, slob_next(next));
		} else
			set_slob(b, units, next);

		if (prev + slob_units(prev) == b) {
			/* merge with the previous block */
			units = slob_units(b) + slob_units(prev);
			set_slob(prev, units, slob_next(b));
		} else
			set_slob(prev, slob_units(prev), b);
	}
out:
	up(&sem);
}
