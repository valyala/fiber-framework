#include "private/ff_common.h"

#include "private/ff_threadpool.h"
#include "private/arch/ff_arch_completion_port.h"
#include "private/arch/ff_arch_thread.h"
#include "private/arch/ff_arch_misc.h"
#include "private/arch/ff_arch_lock.h"

struct ff_threadpool
{
	int max_threads_cnt;
	int running_threads_cnt;
	int busy_threads_cnt;
	struct ff_arch_completion_port *completion_port;
	struct ff_arch_lock *lock;
	struct ff_arch_thread **threads;
};

struct threadpool_task
{
	ff_threadpool_func func;
	void *ctx;
};

static void generic_threadpool_func(void *ctx)
{
	struct ff_threadpool *threadpool;
	
	threadpool = (struct ff_threadpool *) ctx;
	for (;;)
	{
		struct threadpool_task *task;

		ff_arch_lock_lock(threadpool->lock);
		threadpool->busy_threads_cnt--;
		ff_arch_lock_unlock(threadpool->lock);
		task = (struct threadpool_task *) ff_arch_completion_port_get(threadpool->completion_port);
		if (task == NULL)
		{
			break;
		}
		ff_arch_lock_lock(threadpool->lock);
		threadpool->busy_threads_cnt++;
		ff_arch_lock_unlock(threadpool->lock);

		task->func(task->ctx);
		ff_free(task);
	}
}

static void add_worker_thread(struct ff_threadpool *threadpool)
{
	struct ff_arch_thread *worker_thread = ff_arch_thread_create(generic_threadpool_func, 0);
	threadpool->threads[threadpool->running_threads_cnt] = worker_thread;
	threadpool->running_threads_cnt++;
	threadpool->busy_threads_cnt++;
	ff_arch_thread_start(worker_thread, threadpool);
}

struct ff_threadpool *ff_threadpool_create(int max_threads_cnt)
{
	struct ff_threadpool *threadpool = (struct ff_threadpool *) ff_malloc(sizeof(*threadpool));
	int cpus_cnt = ff_arch_misc_get_cpus_cnt();

	threadpool->max_threads_cnt = max_threads_cnt;
	threadpool->running_threads_cnt = 0;
	threadpool->busy_threads_cnt = 0;
	threadpool->completion_port = ff_arch_completion_port_create(cpus_cnt);
	threadpool->lock = ff_arch_lock_create();
	threadpool->threads = (struct ff_arch_thread **) ff_malloc(sizeof(*threadpool->threads) * max_threads_cnt);

	return threadpool;
}

void ff_threadpool_delete(struct ff_threadpool *threadpool)
{
	int i;
	for (i = 0; i < threadpool->running_threads_cnt; i++)
	{
		ff_arch_completion_port_put(threadpool->completion_port, NULL);
	}
	for (i = 0; i < threadpool->running_threads_cnt; i++)
	{
		struct ff_arch_thread *thread = threadpool->threads[i];
		ff_arch_thread_join(thread);
		ff_arch_thread_delete(thread);
	}
	ff_free(threadpool->threads);
	ff_arch_lock_delete(threadpool->lock);
	ff_arch_completion_port_delete(threadpool->completion_port);
	ff_free(threadpool);
}

void ff_threadpool_execute(struct ff_threadpool *threadpool, ff_threadpool_func func, void *ctx)
{
	struct threadpool_task *task = (struct threadpool_task *) ff_malloc(sizeof(*task));
	task->func = func;
	task->ctx = ctx;

	ff_arch_completion_port_put(threadpool->completion_port, task);

	ff_arch_lock_lock(threadpool->lock);
	if (threadpool->running_threads_cnt < threadpool->max_threads_cnt)
	{
		if (threadpool->busy_threads_cnt == threadpool->running_threads_cnt)
		{
			add_worker_thread(threadpool);
		}
	}
	ff_arch_lock_unlock(threadpool->lock);
}
