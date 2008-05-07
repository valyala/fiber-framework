#include "private/ff_common.h"

#include "private/ff_core.h"
#include "private/ff_fiber.h"
#include "private/ff_threadpool.h"
#include "private/ff_fiberpool.h"
#include "private/ff_queue.h"
#include "private/ff_mutex.h"
#include "private/arch/ff_arch_completion_port.h"
#include "private/arch/ff_arch_tcp.h"
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

static int must_stop;
static struct ff_fiber *current_fiber;
static struct ff_arch_completion_port *completion_port;
static struct ff_queue *completion_packets;
static struct ff_threadpool *threadpool;
static struct ff_fiberpool *fiberpool;
static struct ff_queue *timeout_operations;
static struct ff_mutex *timeout_operations_mutex;
static struct ff_fiber *timeout_checker_fiber;

static void generic_threadpool_func(void *ctx)
{
	struct generic_threadpool_data *data;
	
	data = (struct generic_threadpool_data *) ctx;
	data->func(data->ctx);
	ff_core_schedule_remote_fiber(data->fiber);
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
	ff_core_schedule_local_fiber(fiber);
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
	while (!must_stop)
	{
		struct timeout_checker_data timeout_checker_data;

		timeout_checker_data.current_time = ff_arch_misc_get_current_time();
		ff_mutex_lock(timeout_operations_mutex);
		ff_queue_for_each(timeout_operations, timeout_operations_visitor_func, &timeout_checker_data);
		ff_mutex_unlock(timeout_operations_mutex);
		internal_sleep(TIMEOUT_CHECKER_INTERVAL);
	}
}

void ff_core_initialize()
{
	must_stop = 0;
	current_fiber = ff_fiber_initialize();
	completion_port = ff_arch_completion_port_create(COMPLETION_PORT_CONCURRENCY);
	completion_packets = ff_queue_create();
	threadpool = ff_threadpool_create(MAX_THREADPOOL_SIZE);
	fiberpool = ff_fiberpool_create(MAX_FIBERPOOL_SIZE);
	timeout_operations = ff_queue_create();
	timeout_operations_mutex = ff_mutex_create();
	timeout_checker_fiber = ff_fiber_create(timeout_checker_func, 0);
	ff_arch_tcp_initialize(completion_port);
	ff_fiber_start(timeout_checker_fiber, NULL);
}

void ff_core_shutdown()
{
	must_stop = 1;
	ff_fiber_join(timeout_checker_fiber);
	ff_fiber_delete(timeout_checker_fiber);
	ff_arch_tcp_shutdown();
	ff_mutex_delete(timeout_operations_mutex);
	ff_queue_delete(timeout_operations);
	ff_fiberpool_delete(fiberpool);
	ff_threadpool_delete(threadpool);
	ff_queue_delete(completion_packets);
	ff_arch_completion_port_delete(completion_port);
	ff_fiber_shutdown();
}

void ff_core_sleep(int interval)
{
	ff_core_do_timeout_operation(interval, sleep_timeout_func, NULL);
}

void ff_core_threadpool_execute(ff_core_threadpool_func func, void *ctx)
{
	struct generic_threadpool_data data;

	data.fiber = current_fiber;
	data.func = func;
	data.ctx = ctx;
	ff_threadpool_execute(threadpool, generic_threadpool_func, &data);
	ff_core_yield_fiber();
}

void ff_core_fiberpool_begin_execute(ff_core_fiberpool_func func, void *ctx)
{
	ff_fiberpool_execute(fiberpool, func, ctx);
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
		cancel_timeout_data.fiber = current_fiber;
		cancel_timeout_data.ctx = ctx;
		cancel_timeout_data.is_expired = 0;

		ff_mutex_lock(timeout_operations_mutex);
		ff_queue_push(timeout_operations, &cancel_timeout_data);
		ff_mutex_unlock(timeout_operations_mutex);
	}
	ff_core_yield_fiber();
	if (timeout > 0)
	{
		int is_removed;

		ff_mutex_lock(timeout_operations_mutex);
		is_removed = ff_queue_remove_entry(timeout_operations, &cancel_timeout_data);
		ff_assert(is_removed);
		ff_mutex_unlock(timeout_operations_mutex);
		is_success = !cancel_timeout_data.is_expired;
	}
	return is_success;
}

void ff_core_schedule_local_fiber(struct ff_fiber *fiber)
{
	ff_queue_push(completion_packets, fiber);
}

void ff_core_schedule_remote_fiber(struct ff_fiber *fiber)
{
	ff_arch_completion_port_put(completion_port, fiber);
}

void ff_core_yield_fiber()
{
	int is_empty;
	
	is_empty = ff_queue_is_empty(completion_packets);
	if (!is_empty)
	{
		current_fiber = (struct ff_fiber *) ff_queue_front(completion_packets);
		ff_assert(current_fiber != NULL);
		ff_queue_pop(completion_packets);
	}
	else
	{
		current_fiber = (struct ff_fiber *) ff_arch_completion_port_get(completion_port);
		ff_assert(current_fiber != NULL);
	}
	ff_fiber_switch(current_fiber);
}

struct ff_fiber *ff_core_get_current_fiber()
{
	return current_fiber;
}
