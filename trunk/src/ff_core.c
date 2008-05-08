#include "private/ff_common.h"

#include "private/ff_core.h"
#include "private/ff_fiber.h"
#include "private/ff_threadpool.h"
#include "private/ff_fiberpool.h"
#include "private/ff_queue.h"
#include "private/ff_mutex.h"
#include "private/arch/ff_arch_completion_port.h"
#include "private/arch/ff_arch_misc.h"

static const int COMPLETION_PORT_CONCURRENCY = 1;
static const int MAX_THREADPOOL_SIZE = 100;
static const int MAX_FIBERPOOL_SIZE = 100;
static const int TIMEOUT_CHECKER_INTERVAL = 500;

struct cancel_timeout_data
{
	int64_t expiration_time;
	ff_core_cancel_timeout_func cancel_timeout_func;
	struct ff_fiber *fiber;
	void *ctx;
	int is_expired;
};

struct timeout_checker_data
{
	int64_t current_time;
};

struct threadpool_sleep_data
{
	int interval;
};

struct generic_threadpool_data
{
	struct ff_fiber *fiber;
	ff_core_threadpool_func func;
	void *ctx;
};

struct core_data
{
	int must_stop;
	struct ff_fiber *current_fiber;
	struct ff_arch_completion_port *completion_port;
	struct ff_queue *pending_fibers;
	struct ff_threadpool *threadpool;
	struct ff_fiberpool *fiberpool;
	struct ff_queue *timeout_operations;
	struct ff_mutex *timeout_operations_mutex;
	struct ff_fiber *timeout_checker_fiber;
};

static struct core_data core_ctx;

static void generic_threadpool_func(void *ctx)
{
	struct generic_threadpool_data *data;

	data = (struct generic_threadpool_data *) ctx;
	data->func(data->ctx);
	ff_arch_completion_port_put(core_ctx.completion_port, data->fiber);
}

static void threadpool_sleep(void *ctx)
{
	struct threadpool_sleep_data *data;

	data = (struct threadpool_sleep_data *) ctx;
	ff_arch_misc_sleep(data->interval);
}

static void internal_sleep(int interval)
{
	struct threadpool_sleep_data data;

	data.interval = interval;
	ff_core_threadpool_execute(threadpool_sleep, &data);
}

static void sleep_timeout_func(struct ff_fiber *fiber, void *ctx)
{
	ff_core_schedule_fiber(fiber);
}

static void timeout_operations_visitor_func(void *data, void *ctx)
{
	struct cancel_timeout_data *cancel_timeout_data;
	struct timeout_checker_data *timeout_checker_data;
	
	cancel_timeout_data = (struct cancel_timeout_data *) data;
	timeout_checker_data = (struct timeout_checker_data *) ctx;
	if (!cancel_timeout_data->is_expired)
	{
		if (timeout_checker_data->current_time > cancel_timeout_data->expiration_time)
		{
			cancel_timeout_data->cancel_timeout_func(cancel_timeout_data->fiber, cancel_timeout_data->ctx);
			cancel_timeout_data->is_expired = 1;
		}
	}
}

static void timeout_checker_func(void *ctx)
{
	while (!core_ctx.must_stop)
	{
		struct timeout_checker_data timeout_checker_data;

		timeout_checker_data.current_time = ff_arch_misc_get_current_time();
		ff_mutex_lock(core_ctx.timeout_operations_mutex);
		ff_queue_for_each(core_ctx.timeout_operations, timeout_operations_visitor_func, &timeout_checker_data);
		ff_mutex_unlock(core_ctx.timeout_operations_mutex);
		internal_sleep(TIMEOUT_CHECKER_INTERVAL);
	}
}

void ff_core_initialize()
{
	core_ctx.must_stop = 0;
	core_ctx.current_fiber = ff_fiber_initialize();
	core_ctx.completion_port = ff_arch_completion_port_create(COMPLETION_PORT_CONCURRENCY);
	core_ctx.pending_fibers = ff_queue_create();
	core_ctx.threadpool = ff_threadpool_create(MAX_THREADPOOL_SIZE);
	core_ctx.fiberpool = ff_fiberpool_create(MAX_FIBERPOOL_SIZE);
	core_ctx.timeout_operations = ff_queue_create();
	core_ctx.timeout_operations_mutex = ff_mutex_create();
	core_ctx.timeout_checker_fiber = ff_fiber_create(timeout_checker_func, 0);
	ff_fiber_start(core_ctx.timeout_checker_fiber, NULL);
	ff_arch_misc_initialize(core_ctx.completion_port);
}

void ff_core_shutdown()
{
	ff_arch_misc_shutdown();
	core_ctx.must_stop = 1;
	ff_fiber_join(core_ctx.timeout_checker_fiber);
	ff_fiber_delete(core_ctx.timeout_checker_fiber);
	ff_mutex_delete(core_ctx.timeout_operations_mutex);
	ff_queue_delete(core_ctx.timeout_operations);
	ff_fiberpool_delete(core_ctx.fiberpool);
	ff_threadpool_delete(core_ctx.threadpool);
	ff_queue_delete(core_ctx.pending_fibers);
	ff_arch_completion_port_delete(core_ctx.completion_port);
	ff_fiber_shutdown();
}

void ff_core_sleep(int interval)
{
	ff_core_do_timeout_operation(interval, sleep_timeout_func, NULL);
}

void ff_core_threadpool_execute(ff_core_threadpool_func func, void *ctx)
{
	struct generic_threadpool_data data;

	data.fiber = core_ctx.current_fiber;
	data.func = func;
	data.ctx = ctx;
	ff_threadpool_execute(core_ctx.threadpool, generic_threadpool_func, &data);
	ff_core_yield_fiber();
}

void ff_core_fiberpool_begin_execute(ff_core_fiberpool_func func, void *ctx)
{
	ff_fiberpool_execute(core_ctx.fiberpool, func, ctx);
}

int ff_core_do_timeout_operation(int timeout, ff_core_cancel_timeout_func cancel_timeout_func, void *ctx)
{
	int is_success = 1;
	struct cancel_timeout_data cancel_timeout_data;

	if (timeout > 0)
	{
		int64_t current_time;

		current_time = ff_arch_misc_get_current_time();
		cancel_timeout_data.expiration_time = current_time + timeout;
		cancel_timeout_data.cancel_timeout_func = cancel_timeout_func;
		cancel_timeout_data.fiber = core_ctx.current_fiber;
		cancel_timeout_data.ctx = ctx;
		cancel_timeout_data.is_expired = 0;

		ff_mutex_lock(core_ctx.timeout_operations_mutex);
		ff_queue_push(core_ctx.timeout_operations, &cancel_timeout_data);
		ff_mutex_unlock(core_ctx.timeout_operations_mutex);
	}
	ff_core_yield_fiber();
	if (timeout > 0)
	{
		int is_removed;

		ff_mutex_lock(core_ctx.timeout_operations_mutex);
		is_removed = ff_queue_remove_entry(core_ctx.timeout_operations, &cancel_timeout_data);
		ff_assert(is_removed);
		ff_mutex_unlock(core_ctx.timeout_operations_mutex);
		is_success = !cancel_timeout_data.is_expired;
	}
	return is_success;
}

void ff_core_schedule_fiber(struct ff_fiber *fiber)
{
	ff_queue_push(core_ctx.pending_fibers, fiber);
}

void ff_core_yield_fiber()
{
	int is_empty;

	is_empty = ff_queue_is_empty(core_ctx.pending_fibers);
	if (!is_empty)
	{
		core_ctx.current_fiber = (struct ff_fiber *) ff_queue_front(core_ctx.pending_fibers);
		ff_assert(core_ctx.current_fiber != NULL);
		ff_queue_pop(core_ctx.pending_fibers);
	}
	else
	{
		core_ctx.current_fiber = (struct ff_fiber *) ff_arch_completion_port_get(core_ctx.completion_port);
		ff_assert(core_ctx.current_fiber != NULL);
	}
	ff_fiber_switch(core_ctx.current_fiber);
}

struct ff_fiber *ff_core_get_current_fiber()
{
	return core_ctx.current_fiber;
}
