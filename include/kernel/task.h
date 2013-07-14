#ifndef _TASK_H_
#define _TASK_H_

#include "kernel/list.h"
#include "arch/ops.h"

#define INVALID_PRIORITY 0
#define MAX_PRIORITY	 8

enum task_state {                // Task state values
        
        RUNNING    = 0,          // Task is runnable or running
        SLEEPING   = 1,          // Task is waiting for something to happen
        SUSPENDED  = 2,          // Suspend count is non-zero
        CREATING   = 4,          // Task is being created
        EXITED     = 8,         // Task has exited
};


typedef int (*task_routine)(void *arg);

typedef struct task {
	struct list_head list;
	unsigned int sp;
	
	unsigned int priority;
	enum task_state state;

	void *stack;
	int stack_size;

	task_routine entry;
	void *args;

	int ret;

	char name[32];
} task_t;

void initial_task_func(void);
int task_create(task_t *task, task_routine entry, void *args);
void task_schedule(void);
void task_create_init(void);
void task_init(void);

static inline void enter_critical_section(void)
{
	arch_disable_ints();
}

static inline void exit_critical_section(void)
{
	arch_enable_ints();
}

#endif
