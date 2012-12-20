#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

enum handler_return {
	INT_NO_RESCHEDULE = 0,
	INT_RESCHEDULE,
};

int mask_interrupt(unsigned int vector);
int unmask_interrupt(unsigned int vector);

typedef enum handler_return (*int_handler)(void *arg);

void register_int_handler(unsigned int vector, int_handler handler, void *arg);



#endif
