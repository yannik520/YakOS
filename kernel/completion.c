#include <kernel/task.h>
#include <kernel/completion.h>
#define UINT_MAX	(~0U)

void complete(struct completion *x)
{
	x->done++;
	__wake_up_locked(&x->wait, 1);
}

void complete_all(struct completion *x)
{
	x->done += UINT_MAX/2;
	__wake_up_locked(&x->wait, 0);
}

static inline long
do_wait_for_common(struct completion *x,
		   long (*action)(long), long timeout, int state)
{
	if (!x->done) {
		DECLARE_WAITQUEUE(wait, current_task);

		__add_wait_queue_tail_exclusive(&x->wait, &wait);
		do {
			set_current_state(state);
			timeout = action(timeout);
		} while (!x->done && timeout);
		__remove_wait_queue(&x->wait, &wait);
		if (!x->done)
			return timeout;
	}
	x->done--;
	return timeout ? 0 : 1;
}

static inline long
__wait_for_common(struct completion *x,
		  long (*action)(long), long timeout, int state)
{
	down(&x->wait.lock);
	timeout = do_wait_for_common(x, action, timeout, state);
	up(&x->wait.lock);
	return timeout;
}

static long
wait_for_common(struct completion *x, long timeout, int state)
{
	return __wait_for_common(x, schedule_timeout, timeout, state);
}

void wait_for_completion(struct completion *x)
{
	wait_for_common(x, MAX_SCHEDULE_TIMEOUT, SLEEPING);
}

unsigned long
wait_for_completion_timeout(struct completion *x, unsigned long timeout)
{
	return wait_for_common(x, timeout, SLEEPING);
}

bool completion_done(struct completion *x)
{
	int ret = 1;

	down(&x->wait.lock);
	if (!x->done)
		ret = 0;
	up(&x->wait.lock);
	return ret;
}

