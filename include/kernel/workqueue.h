#ifndef _WORKQUEUE_H_
#define _WORKQUEUE_H_

#include <kernel/list.h>
#include <kernel/timer.h>

#define WQ_STACK_SIZE		(0x2000)

struct workqueue_struct;

struct work_struct {
	unsigned long		 pending;
	struct list_head	 entry;
	void (*func)(void *);
	void			*data;
	void			*wq_data;
	timer_t			 timer;
};

#define __WORK_INITIALIZER(n, f, d) {			\
	.entry = { &(n).entry, &(n).entry },		\
	.func  = (f),					\
	.data  = (d),					\
	.timer = TIMER_INITIALIZER(NULL, 0, 0, NULL),	\
}

#define DECLARE_WORK(n, f, d)				\
	struct work_struct n = __WORK_INITIALIZER(n, f, d)

#define PREPARE_WORK(_work, _func, _data)		\
	do {						\
		(_work)->func = _func;			\
		(_work)->data = _data;			\
	} while (0)

#define INIT_WORK(_work, _func, _data)			\
	do {						\
		INIT_LIST_HEAD(&(_work)->entry);	\
		(_work)->pending = 0;			\
		PREPARE_WORK((_work), (_func), (_data));\
		init_timer_value(&(_work)->timer);	\
	} while (0)

extern struct workqueue_struct *create_workqueue(const char *name);
extern void destroy_workqueue(struct workqueue_struct *wq);
extern int queue_work(struct workqueue_struct *wq, struct work_struct *work);
extern int queue_delayed_work(struct workqueue_struct *wq, struct work_struct *work, unsigned long delay);
extern void flush_workqueue(struct workqueue_struct *wq);
extern int schedule_work(struct work_struct *work);
extern int schedule_delayed_work(struct work_struct *work, unsigned long delay);
extern void flush_scheduled_work(void);
extern int current_is_keventd(void);
static inline void cancel_delayed_work(struct work_struct *work)
{
	timer_delete(&work->timer);
}

#endif
