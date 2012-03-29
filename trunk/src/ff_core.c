#include "private/ff_common.h"

#include "private/ff_core.h"
#include "private/ff_log.h"
#include "private/ff_fiber.h"
#include "private/ff_threadpool.h"
#include "private/ff_fiberpool.h"
#include "private/ff_stack.h"
#include "private/ff_container.h"
#include "private/ff_mutex.h"
#include "private/ff_semaphore.h"
#include "private/arch/ff_arch_completion_port.h"
#include "private/arch/ff_arch_misc.h"

/**
 * This number must be equal to 1.
 */
#define COMPLETION_PORT_CONCURRENCY  1

/**
 * the maximum number of threads in the threadpool.
 */
#define MAX_THREADPOOL_SIZE 500

/**
 * the maximum number of fibers in the fiberpool.
 */
#define MAX_FIBERPOOL_SIZE 5000

/**
 * interval in milliseconds for timeout checker fiber,
 * which executes cancellation callbacks for timed out operations.
 */
#define TIMEOUT_CHECKER_INTERVAL 100

struct ff_core_timeout_operation_data
{
	int64_t expiration_time;
	struct ff_container_entry *timeout_operation_entry;
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

struct deferred_func_data
{
	ff_core_fiberpool_func func;
	void *ctx;
	struct ff_core_timeout_operation_data *timeout_operation_data;
};

struct core_data
{
	struct ff_arch_completion_port *completion_port;
	struct ff_stack *pending_fibers;
	struct ff_threadpool *threadpool;
	struct ff_fiberpool *fiberpool;
	struct ff_container *timeout_operations;
	struct ff_mutex *timeout_operations_mutex;
	struct ff_semaphore *timeout_operations_semaphore;
	struct ff_fiber *timeout_checker_fiber;
};

static struct core_data core_ctx;
static int is_core_initialized = 0;

static void generic_core_threadpool_func(void *ctx)
{
	struct generic_threadpool_data *data;

	data = (struct generic_threadpool_data *) ctx;
	data->func(data->ctx);
	ff_arch_completion_port_put(core_ctx.completion_port, data->fiber);
}

static void deferred_func(void *ctx)
{
	struct deferred_func_data *data;

	data = (struct deferred_func_data *) ctx;
	data->func(data->ctx);
	ff_core_deregister_timeout_operation(data->timeout_operation_data);
	ff_free(data);
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
	(void)ctx;
	ff_core_schedule_fiber(fiber);
}

static void deferred_timeout_func(struct ff_fiber *fiber, void *ctx)
{
	(void)fiber;
	ff_core_fiberpool_execute_async(deferred_func, ctx);
}

static void timeout_operations_visitor_func(const void *data, void *ctx)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	struct timeout_checker_data *timeout_checker_data;

	timeout_operation_data = (struct ff_core_timeout_operation_data *) data;
	timeout_checker_data = (struct timeout_checker_data *) ctx;
	if (!timeout_operation_data->is_expired)
	{
		if (timeout_checker_data->current_time > timeout_operation_data->expiration_time)
		{
			timeout_operation_data->cancel_timeout_func(timeout_operation_data->fiber, timeout_operation_data->ctx);
			timeout_operation_data->is_expired = 1;
		}
	}
	else
	{
		ff_log_debug(L"timeout operation already expired. timeout_operation_data=%p, timeout_checker_data=%p", timeout_operation_data, timeout_checker_data);
	}
}

static void timeout_checker_func(void *ctx)
{
	(void)ctx;
	for (;;)
	{
		struct timeout_checker_data timeout_checker_data;
		int is_empty;

		ff_semaphore_down(core_ctx.timeout_operations_semaphore);
		is_empty = ff_container_is_empty(core_ctx.timeout_operations);
		if (is_empty)
		{
			break;
		}
		timeout_checker_data.current_time = ff_arch_misc_get_current_time();
		ff_mutex_lock(core_ctx.timeout_operations_mutex);
		ff_container_for_each(core_ctx.timeout_operations, timeout_operations_visitor_func, &timeout_checker_data);
		ff_mutex_unlock(core_ctx.timeout_operations_mutex);
		ff_semaphore_up(core_ctx.timeout_operations_semaphore);

		internal_sleep(TIMEOUT_CHECKER_INTERVAL);
	}
}

void ff_core_initialize(const wchar_t *log_filename)
{
	ff_assert(!is_core_initialized);
	ff_log_initialize(log_filename);
	ff_fiber_initialize();
	core_ctx.completion_port = ff_arch_completion_port_create(COMPLETION_PORT_CONCURRENCY);
	ff_arch_misc_initialize(core_ctx.completion_port);
	core_ctx.pending_fibers = ff_stack_create();
	core_ctx.threadpool = ff_threadpool_create(MAX_THREADPOOL_SIZE);
	core_ctx.fiberpool = ff_fiberpool_create(MAX_FIBERPOOL_SIZE);
	core_ctx.timeout_operations = ff_container_create();
	core_ctx.timeout_operations_mutex = ff_mutex_create();
	core_ctx.timeout_operations_semaphore = ff_semaphore_create(0);
	core_ctx.timeout_checker_fiber = ff_fiber_create(timeout_checker_func, 0);
	ff_fiber_start(core_ctx.timeout_checker_fiber, NULL);
	is_core_initialized = 1;
}

void ff_core_shutdown()
{
	ff_assert(is_core_initialized);
	ff_semaphore_up(core_ctx.timeout_operations_semaphore);
	ff_fiber_join(core_ctx.timeout_checker_fiber);
	ff_fiber_delete(core_ctx.timeout_checker_fiber);
	ff_semaphore_delete(core_ctx.timeout_operations_semaphore);
	ff_mutex_delete(core_ctx.timeout_operations_mutex);
	ff_container_delete(core_ctx.timeout_operations);
	ff_fiberpool_delete(core_ctx.fiberpool);
	ff_threadpool_delete(core_ctx.threadpool);
	ff_stack_delete(core_ctx.pending_fibers);
	ff_arch_misc_shutdown();
	ff_arch_completion_port_delete(core_ctx.completion_port);
	ff_fiber_shutdown();
	ff_log_shutdown();
	is_core_initialized = 0;
}

void ff_core_sleep(int interval)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;

	ff_assert(interval > 0);

	timeout_operation_data = ff_core_register_timeout_operation(interval, sleep_timeout_func, NULL);
	ff_core_yield_fiber();
	ff_core_deregister_timeout_operation(timeout_operation_data);
}

void ff_core_threadpool_execute(ff_core_threadpool_func func, void *ctx)
{
	struct generic_threadpool_data data;

	data.fiber = ff_fiber_get_current();
	data.func = func;
	data.ctx = ctx;
	ff_threadpool_execute_async(core_ctx.threadpool, generic_core_threadpool_func, &data);
	ff_core_yield_fiber();
}

void ff_core_fiberpool_execute_async(ff_core_fiberpool_func func, void *ctx)
{
	ff_fiberpool_execute_async(core_ctx.fiberpool, func, ctx);
}

void ff_core_fiberpool_execute_deferred(ff_core_fiberpool_func func, void *ctx, int interval)
{
	struct deferred_func_data *data;

	ff_assert(interval > 0);

	data = (struct deferred_func_data *) ff_malloc(sizeof(*data));
	data->func = func;
	data->ctx = ctx;
	data->timeout_operation_data = ff_core_register_timeout_operation(interval, deferred_timeout_func, data);
}

struct ff_core_timeout_operation_data *ff_core_register_timeout_operation(int timeout, ff_core_cancel_timeout_func cancel_timeout_func, void *ctx)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	int64_t current_time;

	ff_assert(timeout > 0);

	timeout_operation_data = (struct ff_core_timeout_operation_data *) ff_malloc(sizeof(*timeout_operation_data));
	current_time = ff_arch_misc_get_current_time();
	timeout_operation_data->expiration_time = current_time + timeout;
	timeout_operation_data->cancel_timeout_func = cancel_timeout_func;
	timeout_operation_data->fiber = ff_fiber_get_current();
	timeout_operation_data->ctx = ctx;
	timeout_operation_data->is_expired = 0;

	ff_semaphore_up(core_ctx.timeout_operations_semaphore);
	ff_mutex_lock(core_ctx.timeout_operations_mutex);
	timeout_operation_data->timeout_operation_entry = ff_container_add_entry(core_ctx.timeout_operations, timeout_operation_data);
	ff_mutex_unlock(core_ctx.timeout_operations_mutex);

	return timeout_operation_data;
}

enum ff_result ff_core_deregister_timeout_operation(struct ff_core_timeout_operation_data *timeout_operation_data)
{
	enum ff_result result;

	ff_mutex_lock(core_ctx.timeout_operations_mutex);
	ff_container_remove_entry(timeout_operation_data->timeout_operation_entry);
	ff_mutex_unlock(core_ctx.timeout_operations_mutex);
	ff_semaphore_down(core_ctx.timeout_operations_semaphore);

	result = timeout_operation_data->is_expired ? FF_FAILURE : FF_SUCCESS;
	ff_free(timeout_operation_data);
	return result;
}

void ff_core_schedule_fiber(struct ff_fiber *fiber)
{
	ff_stack_push(core_ctx.pending_fibers, fiber);
}

void ff_core_yield_fiber()
{
	int is_empty;
	struct ff_fiber *next_fiber = NULL;

	is_empty = ff_stack_is_empty(core_ctx.pending_fibers);
	if (!is_empty)
	{
		ff_stack_top(core_ctx.pending_fibers, (const void **) &next_fiber);
		ff_assert(next_fiber != NULL);
		ff_stack_pop(core_ctx.pending_fibers);
	}
	else
	{
		ff_arch_completion_port_get(core_ctx.completion_port, (const void **) &next_fiber);
		ff_assert(next_fiber != NULL);
	}
	ff_fiber_switch(next_fiber);
}
