#include "private/ff_common.h"

#include "private/ff_fiberpool.h"
#include "private/ff_blocking_queue.h"
#include "private/ff_fiber.h"

static const int PENDING_TASKS_QUEUE_MAX_SIZE = 1;

struct ff_fiberpool
{
	struct ff_blocking_queue *pending_tasks;
	struct ff_fiber **fibers;
	int max_fibers_cnt;
	int running_fibers_cnt;
	int busy_fibers_cnt;
};

struct fiberpool_task
{
	ff_fiberpool_func func;
	void *ctx;
};

static void generic_fiberpool_func(void *ctx)
{
	struct ff_fiberpool *fiberpool;

	fiberpool = (struct ff_fiberpool *) ctx;
	for (;;)
	{
		struct fiberpool_task *task;

		fiberpool->busy_fibers_cnt--;
		task = (struct fiberpool_task *) ff_blocking_queue_get(fiberpool->pending_tasks);
		fiberpool->busy_fibers_cnt++;
		if (task == NULL)
		{
			break;
		}
		task->func(task->ctx);
		ff_free(task);
	}
}

static void add_worker_fiber(struct ff_fiberpool *fiberpool)
{
	struct ff_fiber *worker_fiber;

	worker_fiber = ff_fiber_create(generic_fiberpool_func, 0);
	fiberpool->fibers[fiberpool->running_fibers_cnt] = worker_fiber;
	fiberpool->running_fibers_cnt++;
	fiberpool->busy_fibers_cnt++;
	ff_fiber_start(worker_fiber, fiberpool);
}

struct ff_fiberpool *ff_fiberpool_create(int max_fibers_cnt)
{
	struct ff_fiberpool *fiberpool;

	ff_assert(max_fibers_cnt > 0);

	fiberpool = (struct ff_fiberpool *) ff_malloc(sizeof(*fiberpool));
	fiberpool->pending_tasks = ff_blocking_queue_create(PENDING_TASKS_QUEUE_MAX_SIZE);
	fiberpool->fibers = (struct ff_fiber **) ff_malloc(sizeof(*fiberpool->fibers) * max_fibers_cnt);
	fiberpool->max_fibers_cnt = max_fibers_cnt;
	fiberpool->running_fibers_cnt = 0;
	fiberpool->busy_fibers_cnt = 0;

	return fiberpool;
}

void ff_fiberpool_delete(struct ff_fiberpool *fiberpool)
{
	int i;

	for (i = 0; i < fiberpool->running_fibers_cnt; i++)
	{
		ff_blocking_queue_put(fiberpool->pending_tasks, NULL);
	}
	for (i = 0; i < fiberpool->running_fibers_cnt; i++)
	{
		struct ff_fiber *fiber;

		fiber = fiberpool->fibers[i];
		ff_fiber_join(fiber);
		ff_fiber_delete(fiber);
	}
	ff_free(fiberpool->fibers);
	ff_blocking_queue_delete(fiberpool->pending_tasks);
	ff_free(fiberpool);
}

void ff_fiberpool_execute(struct ff_fiberpool *fiberpool, ff_fiberpool_func func, void *ctx)
{
	struct fiberpool_task *task;

	task = (struct fiberpool_task *) ff_malloc(sizeof(*task));
	task->func = func;
	task->ctx = ctx;
	ff_blocking_queue_put(fiberpool->pending_tasks, task);

	if (fiberpool->running_fibers_cnt < fiberpool->max_fibers_cnt)
	{
		if (fiberpool->busy_fibers_cnt == fiberpool->running_fibers_cnt)
		{
			add_worker_fiber(fiberpool);
		}
	}
}
