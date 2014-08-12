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
	unsigned int		size;

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

static struct alloctor_info alloctor[] = {
	{MIN_POWER_SLAB, MAX_POWER_SLAB, GROUP_NUM_SLAB, free_slab_area},
	{MIN_POWER_PAGE, MAX_POWER_PAGE, GROUP_NUM_PAGE, free_page_area}
};

void kmalloc_init(unsigned int *addr, unsigned int size)
{
	int i;

	if (NULL == addr) {
		printk("%s %d: addr is null!\n", __FUNCTION__, __LINE__);
		return;
	}

	/* Align to page size */
	addr = (unsigned int *)(((unsigned int)addr + PAGE_SIZE - 1) & PAGE_MASK);

	for (i = 0; i < (int32_t)GROUP_NUM_SLAB; i++)
	{
		INIT_LIST_HEAD(&free_slab_area[i].list);
	}

	for (i = 0; i < (int32_t)GROUP_NUM_PAGE; i++)
	{
		INIT_LIST_HEAD(&free_page_area[i].list);
	}

	list_add_tail((struct list_head *)addr, &free_slab_area[GROUP_NUM_SLAB - 1].list);
	list_add_tail((struct list_head *)((uint32_t)addr + (1<<MAX_POWER_SLAB)), &free_page_area[GROUP_NUM_PAGE - 1].list);
}

static unsigned int round2power(unsigned int num)
{
	unsigned int count = 0;

	while (num)
	{
		num >>= 1;
		count++;
	}
	return count;
}

void *kmalloc(unsigned int size)
{
	int			 i;
	unsigned int		 idx;
	unsigned int		*mem = NULL;
	struct alloctor_info	*cur_alloctor;

	if (size == 0)
		return NULL;

	idx = round2power(size + sizeof(mem_head));

	idx = MAX(idx, MIN_POWER_SLAB);

	if (idx > MAX_POWER_PAGE)
		return NULL;

	/* Select alloctor by size */
	for (i=0; i<(int32_t)(sizeof(alloctor)/sizeof(struct alloctor_info) - 1); i++) {
		if (idx < alloctor[i+1].min_power) {
			break;
		}
	}

	cur_alloctor = &alloctor[i];

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
	unsigned int		 mem_size;	//power index, the real size eque 1<<mem_size
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
	for (i=0; i<(int32_t)(sizeof(alloctor)/sizeof(struct alloctor_info) - 1); i++) {
		if (idx < alloctor[i+1].min_power) {
			break;
		}
	}

	cur_alloctor = &alloctor[i];

	for ( i = mem_size - cur_alloctor->min_power; i< (int32_t)cur_alloctor->group_num; i++)
	{
		list_for_each(pos, &cur_alloctor->area[i].list)
		{

			if ((unsigned int)cur_mem + (1 << mem_size) == (unsigned int)pos) //forward combine
			{
				mem_size += 1;
				combine	  = 1;

				list_del(pos);
				break;
			}
			else if ((unsigned int)pos + (1 << mem_size) == (unsigned int)cur_mem) //backword combine
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
