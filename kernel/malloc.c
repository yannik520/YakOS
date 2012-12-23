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

#include "kernel/malloc.h"
#include "kernel/list.h"
#include "kernel/type.h"

typedef struct mem_head
{
	struct list_head list;
	//just save the power index,the real_size = 1 << size
	unsigned int size;

}mem_head;

#define MIN_POWER	5
#define MAX_POWER	20
#define GROUP_NUM	((MAX_POWER) - (MIN_POWER) + 1)
#define HEADER_SIZE	(sizeof(mem_head))
#define MIN(x, y)	((x) <= (y) ? (x) : (y))
#define MAX(x, y)	((x) <= (y) ? (y) : (x))

static mem_head free_area[GROUP_NUM];

void kmalloc_init(unsigned int *addr, unsigned int size)
{
	int i;

	for (i = 0; i < GROUP_NUM; i++)
	{
		INIT_LIST_HEAD(&free_area[i].list);
	}

	list_add_tail((struct list_head *)addr, &free_area[i-1].list);
}

unsigned int round2power(unsigned int num)
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
	int		 i;
	unsigned int	 idx;
	unsigned int	*mem = NULL;

	if (size == 0)
		return 0;

	idx = round2power(size + sizeof(mem_head));

	idx = MAX(idx, MIN_POWER);

	if (idx > MAX_POWER)
		return NULL;

	/* frome power idx group to find free block, if no,
	   find from  bigger group */
	for (i = idx - MIN_POWER; i < GROUP_NUM; i++)
	{
		mem_head *cur_area = &free_area[i];

		if (!list_empty(&cur_area->list)) //have free block
		{
			mem_head	*tmp;
			unsigned int	 half_size;
			int		 j;

			/* if the block size bigger than needed, looped devide the block into
			   two half, and add second half to lower index group, looped till
			   suitable block gotted*/
			for (j = i; j > idx - MIN_POWER; j--)
			{
				half_size = 1 << (j  - 1 + MIN_POWER);
				tmp = (mem_head *)((unsigned int)cur_area->list.next + half_size);
				list_add_tail(&tmp->list, &free_area[j-1].list);
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

void kfree(unsigned int *addr)
{
	int			 i;
	int			 combine = 0;
	unsigned int		 mem_size; //power index, the real size eque 1<<mem_size
	mem_head		*cur_mem = (mem_head *)addr - 1;
	struct list_head	*pos;

	mem_size = cur_mem->size;

	for ( i = mem_size - MIN_POWER; i< GROUP_NUM; i++)
	{
		list_for_each(pos, &free_area[i].list)
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

	list_add_tail(&cur_mem->list, &free_area[mem_size - MIN_POWER].list);
}
