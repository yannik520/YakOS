#ifndef __MALLOC_H__
#define __MALLOC_H__

struct mem_head
{
	unsigned int flag;
	unsigned int size;
	void *pre;
};


void malloc_init(unsigned int *addr, unsigned int size);
unsigned int * malloc(unsigned int size);
void free(unsigned int *addr);

#define AVAILABLE_FLAG        (0x80000000)
#define AVAILABLE_MASK        (~0x80000000)
#define MEM_FLAG              (0x0000aa55)

#endif
