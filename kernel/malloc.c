#include "kernel/printf.h"
#include "kernel/malloc.h"

struct mem_head *mem_base;

void malloc_init(unsigned int *addr, unsigned int size)
{
	mem_base       = (struct mem_head *)addr;
	mem_base->flag = AVAILABLE_FLAG | MEM_FLAG;
	mem_base->size = size - sizeof(struct mem_head);
	mem_base->pre  = (unsigned int *)0;
}

unsigned int* malloc(unsigned int size)
{
	struct mem_head *current_mem = mem_base;
	struct mem_head *pre_mem;
	int		 delta_size;
	unsigned int	 head_size   = sizeof(struct mem_head);

	while((current_mem->flag & MEM_FLAG) == MEM_FLAG)
	{

		if ((current_mem->flag & AVAILABLE_FLAG) != 0)
		{
			delta_size = current_mem->size - size - head_size;

			if (delta_size >= 0)
			{
				pre_mem		   = current_mem;
				pre_mem->flag	  &= AVAILABLE_MASK;
				pre_mem->size	   = size;
				current_mem	   = pre_mem + head_size + size;
				current_mem->flag  = AVAILABLE_FLAG | MEM_FLAG;
				current_mem->size  = delta_size;
				current_mem->pre   = pre_mem;

				return (unsigned int *)(pre_mem + size);
	
			}
		
		}

		current_mem = current_mem + head_size + current_mem->size;
	}

	return 0;
}

void free(unsigned int *addr)
{
	struct mem_head *current_mem;
	struct mem_head *pre_mem;
	struct mem_head *next_mem;
	unsigned int	 head_size = sizeof(struct mem_head);
	
	if (addr == 0)
	{
		printf("error:addr == 0\n");
		return;
	}

	current_mem = (struct mem_head *)(addr - head_size);
	pre_mem	    = current_mem->pre;
	next_mem    = current_mem + head_size + current_mem->size;

	if ((next_mem->flag & AVAILABLE_FLAG) != 0)
	{
		current_mem->size += next_mem->size + head_size;
		next_mem->flag = 0;
	}
	
	if ((pre_mem != 0) && ((pre_mem->flag & AVAILABLE_FLAG) != 0))
	{
		pre_mem->size += current_mem->size + head_size;
		current_mem->flag = 0;
	}
	else
	{
		current_mem->flag |= AVAILABLE_FLAG;
	}
}
