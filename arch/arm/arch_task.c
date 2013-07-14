#include "string.h"
#include "kernel/task.h"
#include "arch/arch_task.h"

#define ALIGNTO(x, y)    ((x) &= ~((y) - 1))

struct context_switch_frame {
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int lr;
};

void arch_task_initialize(task_t *t)
{

	unsigned int stack_top = (unsigned int)t->stack + t->stack_size;
	unsigned int *addr;
	unsigned int i;

	/* align to 8byte */
	ALIGNTO(stack_top, 8);

	struct context_switch_frame *frame = (struct context_switch_frame *)stack_top;
	frame--;

	memset(frame, 0, sizeof(*frame));

	frame->lr = (unsigned int)&initial_task_func;
	t->sp	  = (unsigned int)(frame);
}

void arch_context_switch(task_t *oldtask, task_t *newtask)
{
	/* printf("oldtask->sp=0x%x, newtask->sp=0x%x\n", */
	/*        oldtask->sp, newtask->sp); */
	arm_context_switch(&oldtask->sp, newtask->sp);
}
