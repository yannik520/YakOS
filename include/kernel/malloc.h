#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "list.h"

void kmalloc_init(unsigned int *addr, unsigned int size);
void *kmalloc(unsigned int size);
void kfree(unsigned int *addr);

#endif
