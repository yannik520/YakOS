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
#include <kernel/task.h>
#include <kernel/printf.h>
#include <kernel/type.h>
#include <kernel/sched.h>

//#define DEBUG           1
#include <kernel/debug.h>

struct list_head	 all_task[MAX_PRIORITY];
unsigned int		 task_bitmap;
extern task_t		*current_task;
extern task_t		*next_task;

void sched_fifo_init (void)
{
	int i;
	
	for (i=0; i< MAX_PRIORITY; i++)
	{
		INIT_LIST_HEAD(&all_task[i]);
	}
}

static void sched_fifo_dump ()
{
	task_t                  *task;
	struct list_head        *list;

	printf("\ntasks:");
	list_for_each(list, &all_task[current_task->priority])
	{
		task = (task_t *)list;
		printf(" %s ", task->name);
	}
	printf("\n");

}

static task_t *_find_next_task(unsigned int priority, struct list_head *start)
{
	task_t           *task;
	struct list_head *iterator;

	list_for_each(iterator, start)
	{
		task = (task_t *)iterator;
		DBG("new_task=%s\n", task->name);
		if ((iterator == &all_task[priority]) || (task->state == BLOCKED))
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if (iterator == start)
	{
		task = NULL;
	}
	
	return task;
}

static void sched_fifo_enqueue_task (task_t *p, int flags)
{
	list_add_tail(&p->list, &all_task[p->priority]);
	task_bitmap |= 1 << p->priority;
}

static void sched_fifo_dequeue_task (task_t *p, int flags)
{
	next_task = _find_next_task(p->priority, &p->list);

	list_del(&p->list);
	if (list_empty(&all_task[p->priority]))
	{
		task_bitmap &= ~(1 << p->priority);
	}
}

static task_t * sched_fifo_pick_next_task (void)
{
	struct list_head    *list     = &current_task->list;
	task_t              *old_task = current_task;
	task_t              *new_task;
	int                  i;
	struct list_head    *current_list;

	#ifdef  DEBUG
	sched_fifo_dump();
	#endif

	for (i=0; i<MAX_PRIORITY; i++)
	{
		if ((task_bitmap >> i) & 1)
		{
			if (i != current_task->priority)
			{
				current_list = &all_task[i];
			}
			else
			{
				if ( (current_task->state == EXITED) ||
				     (current_task->state == SLEEPING) )
				{
					if (NULL != next_task)
					{
						DBG("next_task=%s\n", next_task->name);
						new_task = next_task;
						next_task = NULL;
						break;
					}
					else
					{
						continue;
					}
				}

				current_list = list;
			}
			
			new_task = _find_next_task(i, current_list);
			if (NULL == new_task)
			{
				continue;
			}
			break;
		}
	}

	if (MAX_PRIORITY == i)
	{
		return NULL;
	}

	new_task->state = RUNNING;
	current_task    = new_task;

	DBG("old_task=%s, new_task=%s\n", old_task->name, new_task->name);

	return current_task;
}


const struct sched_class sched_class_fifo = {
	 .init		 = sched_fifo_init,
	 .enqueue_task	 = sched_fifo_enqueue_task,
	 .dequeue_task	 = sched_fifo_dequeue_task,
	 .pick_next_task = sched_fifo_pick_next_task,
	 .dump		 = sched_fifo_dump,
};
