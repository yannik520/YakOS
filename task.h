#ifndef _TASK_H_
#define _TASK_H_

#define MAX_TASKS	8

typedef int (*task_routine)(void *arg);

typedef struct task {
	unsigned int sp;

	void *stack;
	int stack_size;

	task_routine entry;
	void *arg;

	int ret;

	char name[32];
} task_t;

void arch_task_initialize(task_routine func, void * arg);
void arch_context_switch(task_t *oldtask, task_t *newtask);
void task_schedule(void);
#endif
