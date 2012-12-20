#ifndef _TASK_H_
#define _TASK_H_

#include "kernel/list.h"
#include "arch/ops.h"

#define MAX_TASKS	8

typedef int (*task_routine)(void *arg);

typedef struct task {
	struct list_head list;
	unsigned int sp;
	
	unsigned int priority;

	void *stack;
	int stack_size;

	task_routine entry;
	void *args;

	int ret;

	char name[32];
} task_t;

void initial_task_func(void);
task_t *task_create(task_routine entry, void *args, unsigned int priority, int stack_size);
void task_schedule(void);

static inline void enter_critical_section(void)
{
	arch_disable_ints();
}

static inline void exit_critical_section(void)
{
	arch_enable_ints();
}

#endif
