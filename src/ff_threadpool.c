#include "private/ff_common.h"

#include "private/ff_threadpool.h"
#include "private/arch/ff_arch_completion_port.h"
#include "private/arch/ff_arch_thread.h"
#include "private/arch/ff_arch_misc.h"
#include "private/arch/ff_arch_mutex.h"

static const int THREADPOOL_THREAD_STACK_SIZE = 0x10000;

struct ff_threadpool
{
	struct ff_arch_completion_port *completion_port;
	struct ff_arch_mutex *mutex;
	struct ff_arch_thread **threads;
	int max_threads_cnt;
	int running_threads_cnt;
	int busy_threads_cnt;
};

struct threadpool_task
{
	ff_threadpool_func func;
	void *ctx;
};

static void generic_threadpool_func(void *ctx)
{
	struct ff_threadpool *threadpool;
	struct ff_arch_completion_port *completion_port;
	struct ff_arch_mutex *mutex;

	threadpool = (struct ff_threadpool *) ctx;
	completion_port = threadpool->completion_port;
	mutex = threadpool->mutex;
	for (;;)
	{
		struct threadpool_task *task;

		ff_arch_mutex_lock(mutex);
		ff_assert(threadpool->busy_threads_cnt > 0);
		ff_assert(threadpool->busy_threads_cnt <= threadpool->running_threads_cnt);
		ff_assert(threadpool->running_threads_cnt <= threadpool->max_threads_cnt);
		threadpool->busy_threads_cnt--;
		ff_arch_mutex_unlock(mutex);
		ff_arch_completion_port_get(completion_port, (const void **) &task);
		if (task == NULL)
		{
			break;
		}
		ff_arch_mutex_lock(mutex);
		threadpool->busy_threads_cnt++;
		ff_arch_mutex_unlock(mutex);

		task->func(task->ctx);
		ff_free(task);
	}
	ff_arch_mutex_lock(mutex);
	threadpool->running_threads_cnt--;
	ff_arch_mutex_unlock(mutex);
}

static void add_worker_thread(struct ff_threadpool *threadpool)
{
	struct ff_arch_thread *worker_thread;

	worker_thread = ff_arch_thread_create(generic_threadpool_func, THREADPOOL_THREAD_STACK_SIZE);

	threadpool->threads[threadpool->running_threads_cnt] = worker_thread;
	threadpool->running_threads_cnt++;
	threadpool->busy_threads_cnt++;

	ff_arch_thread_start(worker_thread, threadpool);
}

struct ff_threadpool *ff_threadpool_create(int max_threads_cnt)
{
	struct ff_threadpool *threadpool;
	int cpus_cnt;

	ff_assert(max_threads_cnt > 0);

	cpus_cnt = ff_arch_misc_get_cpus_cnt();
	threadpool = (struct ff_threadpool *) ff_malloc(sizeof(*threadpool));
	threadpool->completion_port = ff_arch_completion_port_create(cpus_cnt);
	threadpool->mutex = ff_arch_mutex_create();
	threadpool->threads = (struct ff_arch_thread **) ff_calloc(max_threads_cnt, sizeof(threadpool->threads[0]));
	threadpool->max_threads_cnt = max_threads_cnt;
	threadpool->running_threads_cnt = 0;
	threadpool->busy_threads_cnt = 0;

	return threadpool;
}

void ff_threadpool_delete(struct ff_threadpool *threadpool)
{
	struct ff_arch_completion_port *completion_port;
	struct ff_arch_thread **threads;
	int i;
	int running_threads_cnt;

	completion_port = threadpool->completion_port;
	running_threads_cnt = threadpool->running_threads_cnt;
	for (i = 0; i < running_threads_cnt; i++)
	{
		ff_arch_completion_port_put(completion_port, NULL);
	}
	threads = threadpool->threads;
	for (i = 0; i < running_threads_cnt; i++)
	{
		struct ff_arch_thread *thread;

		thread = threads[i];
		ff_arch_thread_join(thread);
		ff_arch_thread_delete(thread);
	}
	ff_assert(threadpool->busy_threads_cnt == 0);
	ff_assert(threadpool->running_threads_cnt == 0);

	ff_free(threads);
	ff_arch_mutex_delete(threadpool->mutex);
	ff_arch_completion_port_delete(completion_port);
	ff_free(threadpool);
}

void ff_threadpool_execute_async(struct ff_threadpool *threadpool, ff_threadpool_func func, void *ctx)
{
	struct threadpool_task *task;
	struct ff_arch_mutex *mutex;

	task = (struct threadpool_task *) ff_malloc(sizeof(*task));
	task->func = func;
	task->ctx = ctx;
	ff_arch_completion_port_put(threadpool->completion_port, task);

	mutex = threadpool->mutex;
	ff_arch_mutex_lock(mutex);
	ff_assert(threadpool->busy_threads_cnt >= 0);
	ff_assert(threadpool->busy_threads_cnt <= threadpool->running_threads_cnt);
	ff_assert(threadpool->running_threads_cnt <= threadpool->max_threads_cnt);
	if (threadpool->running_threads_cnt < threadpool->max_threads_cnt)
	{
		if (threadpool->busy_threads_cnt == threadpool->running_threads_cnt)
		{
			add_worker_thread(threadpool);
		}
	}
	else
	{
		ff_log_debug(L"threadpool=%p already has maximum size %d, so it cannot contain new threads", threadpool, threadpool->max_threads_cnt);
	}
	ff_arch_mutex_unlock(mutex);
}
