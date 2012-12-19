#include "kernel/task.h"
#include "arch/arch_task.h"

struct context_switch_frame {
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int ip;
	unsigned int fp;
	unsigned int sp;
	unsigned int lr;
	unsigned int cpsr;
};

void arch_task_initialize(task_t *t)
{

	unsigned int stack_addr = (unsigned int)t->stack;
	unsigned int *addr;
	unsigned int i;

	/* align to 8byte */
	stack_addr &= ~(8 - 1);

	struct context_switch_frame *frame = (struct context_switch_frame *)stack_addr;
	frame--;

	addr = (unsigned int *)frame;
	for (i=0; i<sizeof(*frame); i++)
	  *(addr + i) = 0;

	frame->sp = stack_addr;
	frame->cpsr = 0x13; //svc mode, irq and fiq disabled
	frame->lr = (unsigned int)&initial_task_func;
	
	t->sp = (unsigned int)(frame);
}

void arch_context_switch(task_t *oldtask, task_t *newtask)
{
	printf("arch_context_switch\n");
	printf("oldtask->sp=0x%x, newtask->sp=0x%x.\n",
	       oldtask->sp, newtask->sp);
	arm_context_switch(&oldtask->sp, &newtask->sp);
}
