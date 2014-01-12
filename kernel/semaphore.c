#include <kernel/type.h>
#include <kernel/task.h>
#include <kernel/semaphore.h>
#include <compiler.h>

struct semaphore_waiter {
	struct list_head list;
	task_t *task;
	int up;
};

extern task_t	*current_task;

static inline int __down(struct semaphore *sem)
{
	task_t *task = current_task;
	struct semaphore_waiter waiter;

	list_add_tail(&waiter.list, &sem->wait_list);
	waiter.task = task;
	waiter.up = 0;

	for (;;) {
		set_task_state(task, BLOCKED);
		enter_critical_section();
		task_schedule();
		exit_critical_section();
		if (waiter.up)
			return 0;
	}

	list_del(&waiter.list);
	return -1;
}

static noinline void __up(struct semaphore *sem)
{
	struct semaphore_waiter *waiter = list_first_entry(&sem->wait_list, struct semaphore_waiter, list);
	list_del(&waiter->list);
	waiter->up = 1;
	set_task_state(waiter->task, READY);
}

void down(struct semaphore *sem)
{
	enter_critical_section();
	if (likely(sem->count > 0))
		sem->count--;
	else
		__down(sem);
	exit_critical_section();
}

void up(struct semaphore *sem)
{
	enter_critical_section();
	if (likely(list_empty(&sem->wait_list)))
		sem->count++;
	else
		__up(sem);
	exit_critical_section();
}
