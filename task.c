#include "task.h"
#include "printf.h"
#include "malloc.h"

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

extern unsigned int stack_top;
extern void arm_context_switch(unsigned int *old_sp, unsigned int *new_sp);

task_t *current_task;
task_t *all_task[MAX_TASKS];
int task_index = 0;
int running_index;

static void initial_task_func(void)
{
	int ret;
	
	ret = all_task[running_index]->entry(current_task->arg);
}

void arch_task_initialize(task_routine func, void * arg)
{
	task_t *t;
	unsigned int stack_addr = (int)(&stack_top - (task_index + 1) * 0x400);
	int i;
	unsigned int *addr;

	t = malloc(sizeof(task_t));
	if (t == (task_t *)0)
	{
		printf("alloc task_t error!\n");
		return;
	}

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
	t->entry = func;
	t->arg = arg;

	all_task[task_index] = t;
	task_index++;
}

void arch_context_switch(task_t *oldtask, task_t *newtask)
{
	arm_context_switch(&oldtask->sp, &newtask->sp);
}

void task_schedule(void)
{
	int index = running_index;

	if (running_index < task_index - 1)
		running_index++;
	else
		running_index = 0;
	arch_context_switch(all_task[index], all_task[running_index]);
}
