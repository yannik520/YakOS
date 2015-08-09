#include <kernel/list.h>
#include <kernel/semaphore.h>
#include <kernel/wait_queue.h>
#include <kernel/completion.h>
#include <kernel/task.h>
#include <kernel/workqueue.h>
#include <kernel/debug.h>
#include <kernel/bitops.h>
#include <kernel/timer.h>
#include <mm/malloc.h>

struct workqueue_struct {
	struct semaphore	lock;
	long			remove_sequence;
	long			insert_sequence;
	
	struct list_head	worklist;
	wait_queue_head_t	more_work;
	wait_queue_head_t	work_done;

	task_t			*task;
	struct completion	 exit;
};

int queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	int ret = 0;

	if (!test_and_set_bit(0, &work->pending)) {
		assert(!list_empty(&work->entry));
		work->wq_data = wq;

		down(&wq->lock);
		list_add_tail(&work->entry, &wq->worklist);
		wq->insert_sequence++;
		wake_up(&wq->more_work);
		up(&wq->lock);
		ret = 1;
	}
	
	return ret;
}

static enum handler_return delayed_work_timer_fn(timer_t *timer, unsigned long current_time, void * __data)
{
	struct work_struct *work = (struct work_struct *)__data;
	struct workqueue_struct *wq = work->wq_data;

	down(&wq->lock);
	list_add_tail(&work->entry, &wq->worklist);
	wq->insert_sequence++;
	wake_up(&wq->more_work);
	up(&wq->lock);

	return INT_RESCHEDULE;
}

int queue_delayed_work(struct workqueue_struct *wq,
		       struct work_struct *work, unsigned long delay)
{
	int	 ret   = 0;
	timer_t *timer = &work->timer;

	if (!test_and_set_bit(0, &work->pending)) {
		assert(!list_empty(&work->entry));

		work->wq_data = wq;
		oneshot_timer_add(timer, delay, (timer_function)delayed_work_timer_fn, work);
		ret = 1;
	}

	return ret;
}

static inline void run_workqueue(struct workqueue_struct *wq)
{
	down(&wq->lock);
	while (!list_empty(&wq->worklist)) {
		struct work_struct *work = list_entry(wq->worklist.next, struct work_struct, entry);
		void (*f) (void *) = work->func;
		void *data = work->data;

		list_del_init(wq->worklist.next);
		up(&wq->lock);

		assert(work->wq_data != wq);
		clear_bit(0, &work->pending);
		f(data);

		down(&wq->lock);
		wq->remove_sequence++;
		wake_up(&wq->work_done);
	}
	up(&wq->lock);
}

typedef struct startup_s {
	struct workqueue_struct *wq;
	struct completion	 done;
	const char		*name;
}startup_t;

static int worker_thread(void *__startup)
{
	startup_t		*startup = __startup;
	struct workqueue_struct *wq	 = startup->wq;
	DECLARE_WAITQUEUE(wait, current_task);

	wq->task = current_task;

	complete(&startup->done);

	for (;;) {
		add_wait_queue(&wq->more_work, &wait);

		if (!wq->task)
			break;

		if (list_empty(&wq->worklist))
			schedule_timeout(MAX_SCHEDULE_TIMEOUT);
		remove_wait_queue(&wq->more_work, &wait);

		if (!list_empty(&wq->worklist))
			run_workqueue(wq);
	}
	remove_wait_queue(&wq->more_work, &wait);
	complete(&wq->exit);

	return 0;
}

void flush_workqueue(struct workqueue_struct *wq)
{
	DECLARE_WAITQUEUE(wait, current_task);
	long sequence_needed;

	down(&wq->lock);
	sequence_needed = wq->insert_sequence;

	while (sequence_needed - wq->remove_sequence > 0) {
		add_wait_queue(&wq->work_done, &wait);
		up(&wq->lock);
		schedule_timeout(MAX_SCHEDULE_TIMEOUT);
		down(&wq->lock);
	}
	finish_wait(&wq->work_done, &wait);
	up(&wq->lock);
}

static int create_workqueue_thread(struct workqueue_struct *wq,
	const char *name)
{
	task_t		*task_wq;
	startup_t	 startup;
	int		 ret;

	sema_init(&wq->lock, 1);
	wq->task = NULL;
	wq->insert_sequence = 0;
	wq->remove_sequence = 0;
	INIT_LIST_HEAD(&wq->worklist);
	init_waitqueue_head(&wq->more_work);
	init_waitqueue_head(&wq->work_done);
	init_completion(&wq->exit);

	init_completion(&startup.done);
	startup.wq   = wq;
	startup.name = name;
	task_wq = task_alloc("workqueue", WQ_STACK_SIZE, 1);
	if (NULL == task_wq) {
		return -1;
	}
	ret = task_create(task_wq, worker_thread, &startup);
	if (0 == ret) {
		wait_for_completion(&startup.done);
	}

	return ret;
}

struct workqueue_struct *create_workqueue(const char *name)
{
	struct workqueue_struct *wq;

	wq = (struct workqueue_struct *)kmalloc(sizeof(*wq));
	if (!wq)
		return NULL;

	if (create_workqueue_thread(wq, name) < 0) {
		destroy_workqueue(wq);
		wq = NULL;
	}

	return wq;
}

static void cleanup_workqueue_task(struct workqueue_struct *wq)
{
	if (wq->task) {
		wq->task = NULL;
		wake_up(&wq->more_work);
		wait_for_completion(&wq->exit);
	}
}

void destroy_workqueue(struct workqueue_struct *wq)
{
	flush_workqueue(wq);
	cleanup_workqueue_task(wq);
	kfree((void *)wq);
}

static struct workqueue_struct *keventd_wq;

int schedule_work(struct work_struct *work)
{
	return queue_work(keventd_wq, work);
}

int schedule_delayed_work(struct work_struct *work, unsigned long delay)
{
	return queue_delayed_work(keventd_wq, work, delay);
}

void flush_scheduled_work(void)
{
	flush_workqueue(keventd_wq);
}

int current_is_keventd(void)
{
	return (current_task == keventd_wq->task) ? 1 : 0;
}

void init_workqueues(void)
{
	keventd_wq = create_workqueue("events");
	assert(!keventd_wq);
}
