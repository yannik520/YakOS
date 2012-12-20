#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

typedef enum handler_return {
	INT_NO_RESCHEDULE = 0,
	INT_RESCHEDULE,
}handler_return;

int mask_interrupt(unsigned int vector);
int unmask_interrupt(unsigned int vector);

typedef handler_return (*int_handler)(void *arg);

void platform_init_interrupts(void);
void register_int_handler(unsigned int vector, int_handler handler, void *arg);

#endif
