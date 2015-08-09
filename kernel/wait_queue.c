/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <kernel/wait_queue.h>
#include <kernel/task.h>

void __init_waitqueue_head(wait_queue_head_t *q, const char *name)
{
	sema_init(&q->lock, 1);
	INIT_LIST_HEAD(&q->task_list);
}

void add_wait_queue(wait_queue_head_t *q, wait_queue_t *wait)
{
	wait->flags &= ~WQ_FLAG_EXCLUSIVE;
	down(&q->lock);
	__add_wait_queue(q, wait);
	up(&q->lock);
}

void add_wait_queue_exclusive(wait_queue_head_t *q, wait_queue_t *wait)
{
	wait->flags |= WQ_FLAG_EXCLUSIVE;
	down(&q->lock);
	__add_wait_queue_tail(q, wait);
	up(&q->lock);
}

void remove_wait_queue(wait_queue_head_t *q, wait_queue_t *wait)
{
	__remove_wait_queue(q, wait);
}

static void __wake_up_common(wait_queue_head_t *q, int nr_exclusive)
{
	wait_queue_t *curr, *next;

	list_for_each_entry_safe(curr, next, &q->task_list, task_list) {
		unsigned flags = curr->flags;
		if (curr->func(curr) &&
		    (flags & WQ_FLAG_EXCLUSIVE) && !--nr_exclusive)
			break;
	}
}

void __wake_up(wait_queue_head_t *q, int nr_exclusive)
{
	down(&q->lock);
	__wake_up_common(q, nr_exclusive);
	up(&q->lock);
}

void __wake_up_locked(wait_queue_head_t *q, int nr)
{
	__wake_up_common(q, nr);
}

void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
	set_current_state(RUNNING);

	if (!list_empty_careful(&wait->task_list)) {
		down(&q->lock);
		list_del_init(&wait->task_list);
		up(&q->lock);
	}
}


