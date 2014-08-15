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

/* buddy memory allocation

    block_size:  2^6       2^7         2^8         2^9         2^10      ....         2^15

    free_area:   [0]       [1]         [2]         [3]         [4]                    [9]
	     	  o-------+ -------+   	|	    -------------------------------------+
		       	  |	   |	+------------+					 |
       	       	 +++------v++------v++---------------v++---------------------------------v++-----
       memory: 	 ||| 64   ||| 64   |||     128       |||             256                 |||
	       	 |||alloc |||      |||               |||                                 |||
	       	 +++------+++------+++---------------+++---------------------------------+++-----
*/

#include <arch/mmu.h>
#include <kernel/malloc.h>
#include <kernel/list.h>
#include <kernel/types.h>
#include <kernel/printk.h>
#include <compiler.h>

#define MIN_POWER_SLAB	(5u)
#define MAX_POWER_SLAB	(16u)
#define MIN_POWER_PAGE  (12u)
#define MAX_POWER_PAGE	(20u)
#define GROUP_NUM_SLAB	((MAX_POWER_SLAB) - (MIN_POWER_SLAB) + 1)
#define GROUP_NUM_PAGE	((MAX_POWER_PAGE) - (MIN_POWER_PAGE) + 1)
#define HEADER_SIZE	(sizeof(mem_head))
#define MIN(x, y)	((x) <= (y) ? (x) : (y))
#define MAX(x, y)	((x) <= (y) ? (y) : (x))

typedef enum {
	ALLOC_TYPE_SLAB,
	ALLOC_TYPE_PAGE,
	ALLOC_TYPE_MAX
}ALLOC_TYPE;

typedef struct mem_head
{
	struct list_head	list;
	/*just save the power index,the real_size = 1 << size */
	uint32_t		size;

}mem_head;

struct alloctor_info
{
	uint32_t	 min_power;
	uint32_t	 max_power;
	uint32_t	 group_num;
	mem_head	*area;
};

static mem_head free_slab_area[GROUP_NUM_SLAB];
static mem_head free_page_area[GROUP_NUM_PAGE];

static struct alloctor_info alloctors[] = {
	{MIN_POWER_SLAB, MAX_POWER_SLAB, GROUP_NUM_SLAB, free_slab_area},
	{MIN_POWER_PAGE, MAX_POWER_PAGE, GROUP_NUM_PAGE, free_page_area},
	{0, 0, 0, NULL}
};

#define for_each_alloctor(ptr, alloctors)     for (i=0; ptr=&alloctors[i], i<ALLOC_TYPE_MAX; i++)
#define for_each_group(list, group_num, area) for (j=0; list=&(area)[j].list, j<(int32_t)(group_num); j++)

void kmalloc_init(uint32_t *addr, uint32_t size)
{
	int			 i, j;
	struct alloctor_info	*alloctor;
	struct alloctor_info	*last_alloctor = &alloctors[ALLOC_TYPE_MAX];
	struct list_head	*list;

	if (NULL == addr) {
		printk("%s %d: addr is null!\n", __FUNCTION__, __LINE__);
		return;
	}

	/* Align to page size */
	addr = (uint32_t *)(((uint32_t)addr + PAGE_SIZE - 1) & PAGE_MASK);
	
	for_each_alloctor(alloctor, alloctors) {
		uint32_t	 group_num = alloctor->group_num;
		mem_head	*area	   = alloctor->area;

		for_each_group(list, alloctor->group_num, area) {
			INIT_LIST_HEAD(list);
			list_add_tail((struct list_head *)((uint32_t)addr + ((1 << last_alloctor->max_power) & PAGE_MASK)),
				      &area[group_num - 1].list);
		}
		last_alloctor = alloctor;
	}
}

static uint32_t round2power(uint32_t num)
{
	uint32_t count = 0;

	while (num)
	{
		num >>= 1;
		count++;
	}
	return count;
}

void *kmalloc(uint32_t size)
{
	int			 i;
	uint32_t		 idx;
	uint32_t		*mem = NULL;
	struct alloctor_info	*cur_alloctor;

	if (size == 0)
		return NULL;

	idx = round2power(size + sizeof(mem_head));

	idx = MAX(idx, MIN_POWER_SLAB);

	if (idx > MAX_POWER_PAGE)
		return NULL;

	/* Select alloctor by size */
	for (i=0; i<ALLOC_TYPE_MAX - 1; i++) {
		if (idx < alloctors[i+1].min_power) {
			break;
		}
	}

	cur_alloctor = &alloctors[i];

	/* frome power idx group to find free block, if no,
	   find from  bigger group */
	for (i = idx - cur_alloctor->min_power; i < (int32_t)cur_alloctor->group_num; i++)
	{
		mem_head *cur_area = &cur_alloctor->area[i];

		if (!list_empty(&cur_area->list)) //have free block
		{
			mem_head	*tmp;
			unsigned int	 half_size;
			int		 j;

			/* if the block size bigger than needed, looped devide the block into
			   two half, and add second half to lower index group, looped till
			   suitable block gotted*/
			for (j = i; j > (int)(idx - cur_alloctor->min_power); j--)
			{
				half_size = 1 << (j  - 1 + cur_alloctor->min_power);
				tmp = (mem_head *)((unsigned int)cur_area->list.next + half_size);
				list_add_tail(&tmp->list, &cur_alloctor->area[j-1].list);
			}

			tmp = (mem_head *)cur_area->list.next;
			tmp->size = idx;
			mem = (unsigned int *)(tmp + 1);

			list_del(cur_area->list.next);
			break;
		}
	}

	return mem;
}

void kfree(void *addr)
{
	int			 i;
	int			 combine = 0;
	uint32_t		 mem_size;	//power index, the real size eque 1<<mem_size
	mem_head		*cur_mem = (mem_head *)addr - 1;
	struct list_head	*pos;
	struct alloctor_info	*cur_alloctor;
	uint32_t		 idx;

	mem_size = cur_mem->size;

	idx = round2power(mem_size + sizeof(mem_head));

	idx = MAX(idx, MIN_POWER_SLAB);

	if (idx > MAX_POWER_PAGE)
		return;

	/* Select alloctor by size */
	for (i=0; i<(int32_t)(sizeof(alloctors)/sizeof(struct alloctor_info) - 1); i++) {
		if (idx < alloctors[i+1].min_power) {
			break;
		}
	}

	cur_alloctor = &alloctors[i];

	for ( i = mem_size - cur_alloctor->min_power; i< (int32_t)cur_alloctor->group_num; i++)
	{
		list_for_each(pos, &cur_alloctor->area[i].list)
		{

			if ((uint32_t)cur_mem + (1 << mem_size) == (uint32_t)pos) //forward combine
			{
				mem_size += 1;
				combine	  = 1;

				list_del(pos);
				break;
			}
			else if ((uint32_t)pos + (1 << mem_size) == (uint32_t)cur_mem) //backword combine
			{
				cur_mem	  = (mem_head *)pos;
				mem_size += 1;
				combine	  = 1;

				list_del(pos);
				break;
			}
			else //can't combine
			{
				combine = 0;
			}
		}

		if(combine == 0)
			break;
	}

	list_add_tail(&cur_mem->list, &cur_alloctor->area[mem_size - cur_alloctor->min_power].list);
}
