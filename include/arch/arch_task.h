#ifndef _ARCH_TASK_H_
#define _ARCH_TASK_H_

void arch_task_initialize(task_t *t);
void arch_context_switch(task_t *oldtask, task_t *newtask);

#endif
