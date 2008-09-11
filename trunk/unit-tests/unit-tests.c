#include "ff/ff_common.h"
#include "ff/ff_core.h"
#include "ff/ff_fiber.h"
#include "ff/ff_event.h"
#include "ff/ff_mutex.h"
#include "ff/ff_semaphore.h"
#include "ff/ff_blocking_queue.h"
#include "ff/ff_blocking_stack.h"
#include "ff/ff_pool.h"
#include "ff/ff_file.h"
#include "ff/arch/ff_arch_net_addr.h"
#include "ff/ff_tcp.h"
#include "ff/ff_udp.h"

#include <stdio.h>
#include <string.h>

#define ASSERT(expr, msg) \
	do \
	{ \
		if (!(expr)) \
		{ \
			return ": ASSERT(" #expr ") failed: " msg; \
		} \
	} while (0)

#define RUN_TEST(test_name) \
	do \
	{ \
		const char *msg; \
		msg = test_ ## test_name(); \
		if (msg != NULL) \
		{ \
			return msg; \
		} \
	} while (0)

#define DECLARE_TEST(test_name) static const char *test_ ## test_name()

#define trace_enter() fprintf(stderr, "%s\n", __FUNCTION__)

#pragma region ff_core_tests

DECLARE_TEST(core_init)
{
	trace_enter();
	ff_core_initialize();
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(core_init_multiple)
{
	int i;
	trace_enter();
	for (i = 0; i < 10; i++)
	{
		RUN_TEST(core_init);
	}
	return NULL;
}

DECLARE_TEST(core_sleep)
{
	trace_enter();
	ff_core_initialize();
	ff_core_sleep(100);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(core_sleep_multiple)
{
	int i;

	trace_enter();
	ff_core_initialize();
	for (i = 0; i < 10; i++)
	{
		ff_core_sleep(i * 10 + 1);
	}
	ff_core_shutdown();
	return NULL;
}

static void threadpool_int_increment(void *ctx)
{
	int *a;

	a = (int *) ctx;
	a[1] = a[0] + 1;
}

DECLARE_TEST(core_threadpool_execute)
{
	int a[2];

	trace_enter();
	ff_core_initialize();
	a[0] = 1234;
	a[1] = 4321;
	ff_core_threadpool_execute(threadpool_int_increment, a);
	ASSERT(a[1] == a[0] + 1, "unexpected result");
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(core_threadpool_execute_multiple)
{
	int i;

	trace_enter();
	ff_core_initialize();
	for (i = 0; i < 10; i++)
	{
		int a[2];

		a[0] = i;
		a[1] = i + 5;
		ff_core_threadpool_execute(threadpool_int_increment, a);
		ASSERT(a[1] == a[0] + 1, "unexpected result");
	}
	ff_core_shutdown();
	return NULL;
}

static void fiberpool_int_increment(void *ctx)
{
	int *a;

	a = (int *) ctx;
	(*a)++;
}

DECLARE_TEST(core_fiberpool_execute)
{
	int a = 0;

	trace_enter();
	ff_core_initialize();
	ff_core_fiberpool_execute_async(fiberpool_int_increment, &a);
	ff_core_shutdown();
	ASSERT(a == 1, "unexpected result");
	return NULL;
}

DECLARE_TEST(core_fiberpool_execute_multiple)
{
	int a = 0;
	int i;

	trace_enter();
	ff_core_initialize();
	for (i = 0; i < 10; i++)
	{
		ff_core_fiberpool_execute_async(fiberpool_int_increment, &a);
	}
	ff_core_shutdown();
	ASSERT(a == 10, "unexpected result");
	return NULL;
}

DECLARE_TEST(core_fiberpool_execute_deferred)
{
	int a = 0;

	ff_core_initialize();
	ff_core_fiberpool_execute_deferred(fiberpool_int_increment, &a, 500);
	ff_core_sleep(10);
	ASSERT(a == 0, "unexpected result");
	ff_core_shutdown();
	ASSERT(a == 1, "unexpected result");
	return NULL;
}

DECLARE_TEST(core_fiberpool_execute_deferred_multiple)
{
	int a = 0;
	int i;

	ff_core_initialize();
	for (i = 0; i < 10; i++)
	{
		ff_core_fiberpool_execute_deferred(fiberpool_int_increment, &a, 500);
	}
	ff_core_sleep(10);
	ASSERT(a == 0, "unexpected result");
	ff_core_shutdown();
	ASSERT(a == 10, "unexpected result");
	return NULL;
}

DECLARE_TEST(core_all)
{
	RUN_TEST(core_init);
	RUN_TEST(core_init_multiple);
	RUN_TEST(core_sleep);
	RUN_TEST(core_sleep_multiple);
	RUN_TEST(core_threadpool_execute);
	RUN_TEST(core_threadpool_execute_multiple);
	RUN_TEST(core_fiberpool_execute);
	RUN_TEST(core_fiberpool_execute_multiple);
	RUN_TEST(core_fiberpool_execute_deferred);
	RUN_TEST(core_fiberpool_execute_deferred_multiple);
	return NULL;
}

/* end of ff_core tests */
#pragma endregion

#pragma region ff_fiber tests

static void fiber_func(void *ctx)
{
	int *a;

	a = (int *) ctx;
	(*a)++;
}

DECLARE_TEST(fiber_create_delete)
{
	struct ff_fiber *fiber;

	trace_enter();
	ff_core_initialize();
	fiber = ff_fiber_create(fiber_func, 0);
	ASSERT(fiber != NULL, "unexpected result");
	ff_fiber_delete(fiber);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(fiber_start_join)
{
	struct ff_fiber *fiber;
	int a = 0;

	trace_enter();
	ff_core_initialize();
	fiber = ff_fiber_create(fiber_func, 0x100000);
	ff_fiber_start(fiber, &a);
	ff_fiber_join(fiber);
	ASSERT(a == 1, "unexpected result");
	ff_fiber_delete(fiber);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(fiber_start_multiple)
{
	struct ff_fiber *fibers[10];
	int a = 0;
	int i;

	trace_enter();
	ff_core_initialize();
	for (i = 0; i < 10; i++)
	{
		struct ff_fiber *fiber;

		fiber = ff_fiber_create(fiber_func, 0);
		fibers[i] = fiber;
	}
	for (i = 0; i < 10; i++)
	{
		struct ff_fiber *fiber;

		fiber = fibers[i];
		ff_fiber_start(fiber, &a);
	}
	for (i = 0; i < 10; i++)
	{
		struct ff_fiber *fiber;

		fiber = fibers[i];
		ff_fiber_join(fiber);
	}
	ASSERT(a == 10, "unexpected result");
	for (i = 0; i < 10; i++)
	{
		struct ff_fiber *fiber;

		fiber = fibers[i];
		ff_fiber_delete(fiber);
	}
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(fiber_all)
{
	RUN_TEST(fiber_create_delete);
	RUN_TEST(fiber_start_join);
	RUN_TEST(fiber_start_multiple);
	return NULL;
}

/* end of ff_fiber tests */
#pragma endregion

#pragma region ff_event tests

DECLARE_TEST(event_manual_create_delete)
{
	struct ff_event *event;

	trace_enter();
	ff_core_initialize();

	event = ff_event_create(FF_EVENT_MANUAL);
	ASSERT(event != NULL, "unexpected result");
	ff_event_delete(event);

	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(event_auto_create_delete)
{
	struct ff_event *event;

	trace_enter();
	ff_core_initialize();

	event = ff_event_create(FF_EVENT_AUTO);
	ASSERT(event != NULL, "unexpected result");
	ff_event_delete(event);

	ff_core_shutdown();
	return NULL;
}

static void fiberpool_event_setter(void *ctx)
{
	struct ff_event *event;

	event = (struct ff_event *) ctx;
	ff_event_set(event);
}

DECLARE_TEST(event_manual_basic)
{
	struct ff_event *event;
	int is_set;

	trace_enter();
	ff_core_initialize();
	event = ff_event_create(FF_EVENT_MANUAL);
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "initial event state should be 'not set'");
	ff_core_fiberpool_execute_async(fiberpool_event_setter, event);
	ff_event_wait(event);
	is_set = ff_event_is_set(event);
	ASSERT(is_set, "event should be set by fiberpool_event_setter()");
	ff_event_wait(event);
	is_set = ff_event_is_set(event);
	ASSERT(is_set, "manual event should remain set after ff_event_wait");
	ff_event_set(event);
	is_set = ff_event_is_set(event);
	ASSERT(is_set, "event should be set after ff_event_set");
	ff_event_reset(event);
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "event should be reset by ff_event_reset");
	ff_event_reset(event);
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "event should remain reset after ff_event_set");
	ff_event_delete(event);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(event_auto_basic)
{
	struct ff_event *event;
	int is_set;

	trace_enter();
	ff_core_initialize();
	event = ff_event_create(FF_EVENT_AUTO);
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "initial event state should be 'not set'");
	ff_core_fiberpool_execute_async(fiberpool_event_setter, event);
	ff_event_wait(event);
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "autoreset event should be 'not set' after ff_event_wait");
	ff_event_set(event);
	is_set = ff_event_is_set(event);
	ASSERT(is_set, "event should be set after ff_event_set");
	ff_event_set(event);
	is_set = ff_event_is_set(event);
	ASSERT(is_set, "event should remain set after ff_event_set");
	ff_event_reset(event);
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "event should be 'not set' after ff_event_reset");
	ff_event_reset(event);
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "event should remain 'not set' after ff_event_reset");
	ff_event_delete(event);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(event_manual_timeout)
{
	struct ff_event *event;
	int is_success;
	int is_set;

	trace_enter();
	ff_core_initialize();

	event = ff_event_create(FF_EVENT_MANUAL);
	is_success = ff_event_wait_with_timeout(event, 100);
	ASSERT(!is_success, "event should timeout");
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "event should remain 'not set' after timeout");
	ff_event_set(event);
	is_success = ff_event_wait_with_timeout(event, 100);
	ASSERT(is_success, "event shouldn't timeout");
	is_set = ff_event_is_set(event);
	ASSERT(is_set, "manual event should remain set after ff_event_wait_with_timeout");
	ff_event_delete(event);

	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(event_auto_timeout)
{
	struct ff_event *event;
	int is_success;
	int is_set;

	trace_enter();
	ff_core_initialize();

	event = ff_event_create(FF_EVENT_AUTO);
	is_success = ff_event_wait_with_timeout(event, 100);
	ASSERT(!is_success, "event should timeout");
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "event should remain 'not set' after timeout");
	ff_event_set(event);
	is_success = ff_event_wait_with_timeout(event, 100);
	ASSERT(is_success, "event shouldn't timeout");
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "auto reset event should be 'not set' after ff_event_wait_with_timeout");
	ff_event_delete(event);

	ff_core_shutdown();
	return NULL;
}

static void fiberpool_event_func(void *ctx)
{
	struct ff_event *event, *done_event;
	int *a;

	event = (struct ff_event *) ((void **)ctx)[0];
	done_event = (struct ff_event *) ((void **)ctx)[1];
	a = (int *) ((void **)ctx)[2];

	ff_event_wait(event);
	(*a)++;
	if (*a == 15)
	{
		ff_event_set(done_event);
	}
}

DECLARE_TEST(event_manual_multiple)
{
	struct ff_event *event, *done_event;
	int i;
	int a = 0;
	void *data[3];

	trace_enter();
	ff_core_initialize();
	event = ff_event_create(FF_EVENT_MANUAL);
	done_event = ff_event_create(FF_EVENT_MANUAL);
	data[0] = event;
	data[1] = done_event;
	data[2] = &a;
	for (i = 0; i < 15; i++)
	{
		ff_core_fiberpool_execute_async(fiberpool_event_func, data);
	}
	ASSERT(a == 0, "a shouldn't change while event isn't set");
	ff_event_set(event);
	ff_event_wait(done_event);
	ASSERT(a == 15, "a should set to 15 after done_event set inside fiberpool_event_func");
	ff_event_delete(done_event);
	ff_event_delete(event);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(event_auto_multiple)
{
	struct ff_event *event, *done_event;
	int i;
	int a = 0;
	int is_success;
	void *data[3];

	trace_enter();
	ff_core_initialize();
	event = ff_event_create(FF_EVENT_AUTO);
	done_event = ff_event_create(FF_EVENT_MANUAL);
	data[0] = event;
	data[1] = done_event;
	data[2] = &a;
	for (i = 0; i < 15; i++)
	{
		ff_core_fiberpool_execute_async(fiberpool_event_func, data);
	}
	ASSERT(a == 0, "a shouldn't change while event isn't set");
	for (i = 0; i < 14; i++)
	{
		ff_event_set(event);
		is_success = ff_event_wait_with_timeout(done_event, 1);
		ASSERT(!is_success, "done_event should remain 'not set'");
	}
	ff_event_set(event);
	ff_event_wait(done_event);
	ASSERT(a == 15, "a should have value 15 after done_event set");
	ff_event_delete(done_event);
	ff_event_delete(event);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(event_all)
{
	RUN_TEST(event_manual_create_delete);
	RUN_TEST(event_auto_create_delete);
	RUN_TEST(event_manual_basic);
	RUN_TEST(event_auto_basic);
	RUN_TEST(event_manual_timeout);
	RUN_TEST(event_auto_timeout);
	RUN_TEST(event_manual_multiple);
	RUN_TEST(event_auto_multiple);
	return NULL;
}

/* end of ff_event tests */
#pragma endregion

#pragma region ff_mutex tests

DECLARE_TEST(mutex_create_delete)
{
	struct ff_mutex *mutex;

	trace_enter();
	ff_core_initialize();
	mutex = ff_mutex_create();
	ASSERT(mutex != NULL, "mutex should be initialized");
	ff_mutex_delete(mutex);
	ff_core_shutdown();
	return NULL;
}

static void fiberpool_mutex_func(void *ctx)
{
	struct ff_mutex *mutex;
	struct ff_event *event;
	int *a;

	mutex = (struct ff_mutex *) ((void **)ctx)[0];
	event = (struct ff_event *) ((void **)ctx)[1];
	a = (int *) ((void **)ctx)[2];

	ff_mutex_lock(mutex);
	*a = 123;
	ff_event_set(event);
	ff_core_sleep(100);
	*a = 10;
	ff_mutex_unlock(mutex);
}

DECLARE_TEST(mutex_basic)
{
	void *data[3];
	int a = 0;
	struct ff_mutex *mutex;
	struct ff_event *event;

	trace_enter();
	ff_core_initialize();
	mutex = ff_mutex_create();
	event = ff_event_create(FF_EVENT_AUTO);
	ff_mutex_lock(mutex);
	ff_mutex_unlock(mutex);

	data[0] = mutex;
	data[1] = event;
	data[2] = &a;
	ff_core_fiberpool_execute_async(fiberpool_mutex_func, data);
	ff_event_wait(event);
	ASSERT(a == 123, "a should be 123, because the fiberpool_mutex_func should sleep in the ff_core_sleep");
	ff_mutex_lock(mutex);
	ASSERT(a == 10, "a should be 10, because fiberpool_mutex_func should unlock the mutex after sleep");
	ff_mutex_unlock(mutex);

	ff_event_delete(event);
	ff_mutex_delete(mutex);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(mutex_all)
{
	RUN_TEST(mutex_create_delete);
	RUN_TEST(mutex_basic);
	return NULL;
}

/* end of ff_mutex tests */
#pragma endregion

#pragma region ff_semaphore tests

DECLARE_TEST(semaphore_create_delete)
{
	struct ff_semaphore *semaphore;

	trace_enter();
	ff_core_initialize();
	semaphore = ff_semaphore_create(0);
	ASSERT(semaphore != NULL, "semaphore should be initialized");
	ff_semaphore_delete(semaphore);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(semaphore_basic)
{
	int i;
	int is_success;
	struct ff_semaphore *semaphore;

	trace_enter();
	ff_core_initialize();
	semaphore = ff_semaphore_create(0);
	is_success = ff_semaphore_down_with_timeout(semaphore, 1);
	ASSERT(!is_success, "semaphore with 0 value cannot be down");
	for (i = 0; i < 10; i++)
	{
		ff_semaphore_up(semaphore);
	}
	is_success = ff_semaphore_down_with_timeout(semaphore, 1);
	ASSERT(is_success, "semaphore should be down");
	for (i = 0; i < 9; i++)
	{
		ff_semaphore_down(semaphore);
	}
	is_success = ff_semaphore_down_with_timeout(semaphore, 1);
	ASSERT(!is_success, "semaphore cannot be down");
	ff_semaphore_delete(semaphore);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(semaphore_all)
{
	RUN_TEST(semaphore_create_delete);
	RUN_TEST(semaphore_basic);
	return NULL;
}

/* end of ff_semaphore tests */
#pragma endregion

#pragma region ff_blocking_queue tests

DECLARE_TEST(blocking_queue_create_delete)
{
	struct ff_blocking_queue *queue;

	trace_enter();
	ff_core_initialize();
	queue = ff_blocking_queue_create(10);
	ASSERT(queue != NULL, "queue should be initialized");
	ff_blocking_queue_delete(queue);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(blocking_queue_basic)
{
	int64_t i;
	int is_success;
	int64_t data = 0;
	struct ff_blocking_queue *queue;

	trace_enter();
	ff_core_initialize();
	queue = ff_blocking_queue_create(10);
	for (i = 0; i < 10; i++)
	{
		ff_blocking_queue_put(queue, *(void **)&i);
	}
	is_success = ff_blocking_queue_put_with_timeout(queue, (void *)123, 1);
	ASSERT(!is_success, "queue should be full");
	for (i = 0; i < 10; i++)
	{
		ff_blocking_queue_get(queue, (const void **)&data);
		ASSERT(data == i, "wrong value received from the queue");
	}
	is_success = ff_blocking_queue_get_with_timeout(queue, (const void **)&data, 1);
	ASSERT(!is_success, "queue should be empty");
	ff_blocking_queue_delete(queue);
	ff_core_shutdown();
	return NULL;
}

static void fiberpool_blocking_queue_func(void *ctx)
{
	struct ff_blocking_queue *queue;

	queue = (struct ff_blocking_queue *) ctx;
	ff_blocking_queue_put(queue, (void *)543);
}

DECLARE_TEST(blocking_queue_fiberpool)
{
	int64_t data = 0;
	int is_success;
	struct ff_blocking_queue *queue;

	trace_enter();
	ff_core_initialize();
	queue = ff_blocking_queue_create(1);
	is_success = ff_blocking_queue_get_with_timeout(queue, (const void **)&data, 1);
	ASSERT(!is_success, "queue should be empty");
	ff_core_fiberpool_execute_async(fiberpool_blocking_queue_func, queue);
	ff_blocking_queue_get(queue, (const void **)&data);
	ASSERT(data == 543, "unexpected value received from the queue");
	is_success = ff_blocking_queue_get_with_timeout(queue, (const void **)&data, 1);
	ASSERT(!is_success, "queue shouldn't have values");
	ff_blocking_queue_delete(queue);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(blocking_queue_all)
{
	RUN_TEST(blocking_queue_create_delete);
	RUN_TEST(blocking_queue_basic);
	RUN_TEST(blocking_queue_fiberpool);
	return NULL;
}

/* end of ff_blocking_queue tests */
#pragma endregion

#pragma region ff_blocking_stack tests

DECLARE_TEST(blocking_stack_create_delete)
{
	struct ff_blocking_stack *stack;

	trace_enter();
	ff_core_initialize();
	stack = ff_blocking_stack_create(10);
	ASSERT(stack != NULL, "stack should be initialized");
	ff_blocking_stack_delete(stack);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(blocking_stack_basic)
{
	int64_t i;
	int is_success;
	int64_t data = 0;
	struct ff_blocking_stack *stack;

	trace_enter();
	ff_core_initialize();
	stack = ff_blocking_stack_create(10);
	for (i = 0; i < 10; i++)
	{
		ff_blocking_stack_push(stack, *(void **)&i);
	}
	is_success = ff_blocking_stack_push_with_timeout(stack, (void *)1234, 1);
	ASSERT(!is_success, "stack should be fulll");
	for (i = 9; i >= 0; i--)
	{
		ff_blocking_stack_pop(stack, (const void **)&data);
		ASSERT(data == i, "wrong value retrieved from the stack");
	}
	is_success = ff_blocking_stack_pop_with_timeout(stack, (const void **)&data, 1);
	ASSERT(!is_success, "stack should be empty");
	ff_blocking_stack_delete(stack);
	ff_core_shutdown();
	return NULL;
}

static void fiberpool_blocking_stack_func(void *ctx)
{
	struct ff_blocking_stack *stack;

	stack = (struct ff_blocking_stack *) ctx;
	ff_blocking_stack_push(stack, (void *)543);
}

DECLARE_TEST(blocking_stack_fiberpool)
{
	int64_t data = 0;
	int is_success;
	struct ff_blocking_stack *stack;

	trace_enter();
	ff_core_initialize();
	stack = ff_blocking_stack_create(1);
	is_success = ff_blocking_stack_pop_with_timeout(stack, (const void **)&data, 1);
	ASSERT(!is_success, "stack should be empty");
	ff_core_fiberpool_execute_async(fiberpool_blocking_stack_func, stack);
	ff_blocking_stack_pop(stack, (const void **)&data);
	ASSERT(data == 543, "unexpected value received from the stack");
	is_success = ff_blocking_stack_pop_with_timeout(stack, (const void **)&data, 1);
	ASSERT(!is_success, "stack shouldn't have values");
	ff_blocking_stack_delete(stack);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(blocking_stack_all)
{
	RUN_TEST(blocking_stack_create_delete);
	RUN_TEST(blocking_stack_basic);
	RUN_TEST(blocking_stack_fiberpool);
	return NULL;
}

/* end of ff_blocking_stack tests */
#pragma endregion

#pragma region ff_pool tests

static int pool_entries_cnt = 0;

static void *pool_entry_constructor(void *ctx)
{
	pool_entries_cnt++;
	return (void *)123;
}

static void pool_entry_destructor(void *entry)
{
	pool_entries_cnt--;
}

DECLARE_TEST(pool_create_delete)
{
	struct ff_pool *pool;

	trace_enter();
	ff_core_initialize();
	pool = ff_pool_create(10, pool_entry_constructor, NULL, pool_entry_destructor);
	ASSERT(pool_entries_cnt == 0, "pool should be empty after creation");
	ff_pool_delete(pool);
	ASSERT(pool_entries_cnt == 0, "pool should be empty after deletion");
	ff_core_shutdown();
	return NULL;
}

struct pool_visitor_func_data
{
	int is_acquired;
	int is_success;
};

static void pool_visitor_func(void *entry, void *ctx, int is_acquired)
{
	struct pool_visitor_func_data *data;

	data = (struct pool_visitor_func_data *) ctx;
	if (data->is_success)
	{
		data->is_success = (data->is_acquired == is_acquired);
	}
}

DECLARE_TEST(pool_basic)
{
	struct ff_pool *pool;
	void *entry;
	int i;
	struct pool_visitor_func_data data;

	trace_enter();
	ff_core_initialize();
	pool = ff_pool_create(10, pool_entry_constructor, NULL, pool_entry_destructor);
	for (i = 0; i < 10; i++)
	{
		entry = ff_pool_acquire_entry(pool);
		ASSERT(entry == (void *)123, "unexpected value for the entry");
		ASSERT(pool_entries_cnt == i + 1, "unexpected entries number");
	}

	data.is_acquired = 1;
	data.is_success = 1;
	ff_pool_for_each_entry(pool, pool_visitor_func, &data);
	ASSERT(data.is_success == 1, "unexpected result");

	for (i = 0; i < 10; i++)
	{
		ff_pool_release_entry(pool, (void *)123);
	}

	data.is_acquired = 0;
	data.is_success = 1;
	ff_pool_for_each_entry(pool, pool_visitor_func, &data);
	ASSERT(data.is_success == 1, "unexpected result");

	ff_pool_delete(pool);
	ASSERT(pool_entries_cnt == 0, "pool should be empty after deletion");
	ff_core_shutdown();
	return NULL;
}

static void fiberpool_pool_func(void *ctx)
{
	struct ff_pool *pool;

	pool = (struct ff_pool *) ctx;
	ff_pool_release_entry(pool, (void *)123);
}

DECLARE_TEST(pool_fiberpool)
{
	struct ff_pool *pool;
	void *entry;
	
	trace_enter();
	ff_core_initialize();
	pool = ff_pool_create(1, pool_entry_constructor, NULL, pool_entry_destructor);
	ASSERT(pool_entries_cnt == 0, "pool should be empty after creation");
	entry = ff_pool_acquire_entry(pool);
	ASSERT(entry == (void *)123, "unexpected value received from the pool");
	ASSERT(pool_entries_cnt == 1, "pool should create one entry");
	ff_core_fiberpool_execute_async(fiberpool_pool_func, pool);
	entry = ff_pool_acquire_entry(pool);
	ASSERT(entry == (void *)123, "wrong entry value");
	ASSERT(pool_entries_cnt == 1, "pool should contain one entry");
	ff_pool_release_entry(pool, (void *)123);
	ff_pool_delete(pool);
	ASSERT(pool_entries_cnt == 0, "pool should be empty after deletion");
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(pool_all)
{
	RUN_TEST(pool_create_delete);
	RUN_TEST(pool_basic);
	RUN_TEST(pool_fiberpool);
	return NULL;
}

/* end of ff_pool tests */
#pragma endregion

#pragma region ff_file tests

DECLARE_TEST(file_open_read_fail)
{
	struct ff_file *file;

	trace_enter();
	ff_core_initialize();
	file = ff_file_open(L"unexpected_file.txt", FF_FILE_READ);
	ASSERT(file == NULL, "unexpected file couldn't be opened");
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(file_create_delete)
{
	struct ff_file *file;
	int is_success;

	trace_enter();
	ff_core_initialize();

	file = ff_file_open(L"test.txt", FF_FILE_WRITE);
	ASSERT(file != NULL, "file should be created");
	ff_file_close(file);
	is_success = ff_file_erase(L"test.txt");
	ASSERT(is_success, "file should be deleted");
	is_success = ff_file_erase(L"test.txt");
	ASSERT(!is_success, "file already deleted");

	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(file_basic)
{
	struct ff_file *file;
	int64_t size;
	uint8_t data[] = "hello, world!\n";
	uint8_t buf[sizeof(data) - 1];
	int len;
	int is_equal;
	int is_success;

	trace_enter();
	ff_core_initialize();

	file = ff_file_open(L"test.txt", FF_FILE_WRITE);
	ASSERT(file != NULL, "file should be created");
	size = ff_file_get_size(file);
	ASSERT(size == 0, "file should be empty");
	len = ff_file_write(file, data, sizeof(data) - 1);
	ASSERT(len == sizeof(data) - 1, "all the data should be written to the file");
	len = ff_file_flush(file);
	ASSERT(len != -1, "file should be flushed successfully");
	ff_file_close(file);

	file = ff_file_open(L"test.txt", FF_FILE_READ);
	ASSERT(file != NULL, "file should exist");
	size = ff_file_get_size(file);
	ASSERT(size == sizeof(data) - 1, "wrong file size");
	len = ff_file_read(file, buf, sizeof(data) - 1);
	ASSERT(len == sizeof(data) - 1, "unexpected length of data read from the file");
	is_equal = (memcmp(data, buf, sizeof(data) - 1) == 0);
	ASSERT(is_equal, "wrong data read from the file");
	ff_file_close(file);

	is_success = ff_file_copy(L"test.txt", L"test1.txt");
	ASSERT(is_success, "file should be copied");
	is_success = ff_file_copy(L"test.txt", L"test1.txt");
	ASSERT(!is_success, "cannot copy to existing file");
	is_success = ff_file_move(L"test.txt", L"test2.txt");
	ASSERT(is_success, "file should be moved");
	is_success = ff_file_move(L"test.txt", L"test2.txt");
	ASSERT(!is_success, "cannot move to existing file");

	file = ff_file_open(L"test1.txt", FF_FILE_READ);
	ASSERT(file != NULL, "file copy should exist");
	len = ff_file_read(file, buf, sizeof(data) - 1);
	ASSERT(len = sizeof(data) - 1, "unexpected length of file copy");
	is_equal = (memcmp(data, buf, sizeof(data) - 1) == 0);
	ASSERT(is_equal, "wrong contents of the file copy");
	ff_file_close(file);

	file = ff_file_open(L"test1.txt", FF_FILE_WRITE);
	ASSERT(file != NULL, "file should be truncated");
	size = ff_file_get_size(file);
	ASSERT(size == 0, "truncated file should be empty");
	ff_file_close(file);

	is_success = ff_file_erase(L"test1.txt");
	ASSERT(is_success, "file1 should be deleted");
	is_success = ff_file_erase(L"test2.txt");
	ASSERT(is_success, "file2 should be deleted");

	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(file_all)
{
	RUN_TEST(file_open_read_fail);
	RUN_TEST(file_create_delete);
	RUN_TEST(file_basic);
	return NULL;
}

/* end of ff_file tests */
#pragma endregion

#pragma region ff_arch_net_addr tests

DECLARE_TEST(arch_net_addr_create_delete)
{
	struct ff_arch_net_addr *addr;

	trace_enter();
	ff_core_initialize();
	addr = ff_arch_net_addr_create();
	ASSERT(addr != NULL, "addr should be created");
	ff_arch_net_addr_delete(addr);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(arch_net_addr_resolve_success)
{
	struct ff_arch_net_addr *addr1, *addr2;
	int is_success;
	int is_equal;

	trace_enter();
	ff_core_initialize();
	addr1 = ff_arch_net_addr_create();
	addr2 = ff_arch_net_addr_create();
	is_success = ff_arch_net_addr_resolve(addr1, L"localhost", 80);
	ASSERT(is_success, "localhost address should be resolved successfully");
	is_success = ff_arch_net_addr_resolve(addr2, L"127.0.0.1", 80);
	ASSERT(is_success, "127.0.0.1 address should be resolved successfully");
	is_equal = ff_arch_net_addr_is_equal(addr1, addr2);
	ASSERT(is_equal, "addresses should be equal");
	is_success = ff_arch_net_addr_resolve(addr2, L"123.45.1.1", 0);
	ASSERT(is_success, "numeric address should be resolved successfully");
	is_equal = ff_arch_net_addr_is_equal(addr1, addr2);
	ASSERT(!is_equal, "addresses shouldn't be equivalent");
	ff_arch_net_addr_delete(addr1);
	ff_arch_net_addr_delete(addr2);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(arch_net_addr_resolve_fail)
{
	struct ff_arch_net_addr *addr;
	int is_success;

	trace_enter();
	ff_core_initialize();
	addr = ff_arch_net_addr_create();
	is_success = ff_arch_net_addr_resolve(addr, L"non.existant,address", 123);
	ASSERT(!is_success, "address shouldn't be resolved");
	ff_arch_net_addr_delete(addr);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(arch_net_addr_broadcast)
{
	struct ff_arch_net_addr *addr, *net_mask, *broadcast_addr;
	int is_success;
	int is_equal;

	trace_enter();
	ff_core_initialize();
	addr = ff_arch_net_addr_create();
	net_mask = ff_arch_net_addr_create();
	broadcast_addr = ff_arch_net_addr_create();

	is_success = ff_arch_net_addr_resolve(addr, L"123.45.67.89", 123);
	ASSERT(is_success, "address should be resolved");
	is_success = ff_arch_net_addr_resolve(net_mask, L"255.255.0.0", 0);
	ASSERT(is_success, "net mask should be resolved");
	ff_arch_net_addr_get_broadcast_addr(addr, net_mask, broadcast_addr);
	is_success = ff_arch_net_addr_resolve(addr, L"123.45.255.255", 123);
	ASSERT(is_success, "broadcast address should be resolved");
	is_equal = ff_arch_net_addr_is_equal(addr, broadcast_addr);
	ASSERT(is_equal, "addresses should be equal");

	ff_arch_net_addr_delete(addr);
	ff_arch_net_addr_delete(net_mask);
	ff_arch_net_addr_delete(broadcast_addr);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(arch_net_addr_to_string)
{
	struct ff_arch_net_addr *addr;
	const wchar_t *str;
	int is_success;
	int is_equal;

	ff_core_initialize();
	addr = ff_arch_net_addr_create();
	is_success = ff_arch_net_addr_resolve(addr, L"12.34.56.78", 90);
	ASSERT(is_success, "address should be resolved");
	str = ff_arch_net_addr_to_string(addr);
	is_equal = (wcscmp(L"12.34.56.78:90", str) == 0);
	ASSERT(is_equal, "wrong string");
	ff_arch_net_addr_delete_string(str);
	ff_arch_net_addr_delete(addr);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(arch_net_addr_all)
{
	RUN_TEST(arch_net_addr_create_delete);
	RUN_TEST(arch_net_addr_resolve_success);
	RUN_TEST(arch_net_addr_resolve_fail);
	RUN_TEST(arch_net_addr_broadcast);
	RUN_TEST(arch_net_addr_to_string);
	return NULL;
}

/* end of ff_arch_net_addr tests */
#pragma endregion

#pragma region ff_tcp tests

DECLARE_TEST(tcp_create_delete)
{
	struct ff_tcp *tcp;

	trace_enter();
	ff_core_initialize();
	tcp = ff_tcp_create();
	ASSERT(tcp != NULL, "tcp should be created");
	ff_tcp_delete(tcp);
	ff_core_shutdown();
	return NULL;
}

static void fiberpool_tcp_func(void *ctx)
{
	struct ff_tcp *tcp_server, *tcp_client;
	struct ff_arch_net_addr *client_addr;
	int len;
	uint8_t buf[4];

	tcp_server = (struct ff_tcp *) ctx;
	client_addr = ff_arch_net_addr_create();
	tcp_client = ff_tcp_accept(tcp_server, client_addr);
	if (tcp_client != NULL)
	{
		len = ff_tcp_write(tcp_client, "test", 4);
		len = ff_tcp_flush(tcp_client);
		if (len != -1)
		{
			len = ff_tcp_read(tcp_client, buf, 4);
			if (len == 4)
			{
				int is_equal;
				is_equal = (memcmp("test", buf, 4) == 0);
			}
		}
		ff_tcp_delete(tcp_client);
	}
	ff_arch_net_addr_delete(client_addr);
}

DECLARE_TEST(tcp_basic)
{
	struct ff_tcp *tcp_server, *tcp_client;
	struct ff_arch_net_addr *addr;
	int is_success;
	int len;
	int is_equal;
	uint8_t buf[4];

	trace_enter();
	ff_core_initialize();
	addr = ff_arch_net_addr_create();
	is_success = ff_arch_net_addr_resolve(addr, L"127.0.0.1", 43210);
	ASSERT(is_success, "localhost address should be resolved successfully");
	tcp_server = ff_tcp_create();
	ASSERT(tcp_server != NULL, "server should be created");
	is_success = ff_tcp_bind(tcp_server, addr, FF_TCP_SERVER);
	ASSERT(is_success, "server should be bound to local address");
	ff_core_fiberpool_execute_async(fiberpool_tcp_func, tcp_server);

	tcp_client = ff_tcp_create();
	ASSERT(tcp_client != NULL, "client should be created");
	is_success = ff_tcp_connect(tcp_client, addr);
	ASSERT(is_success, "client should connect to the server");
	len = ff_tcp_read_with_timeout(tcp_client, buf, 4, 100000);
	ASSERT(len == 4, "unexpected data received from the server");
	is_equal = (memcmp(buf, "test", 4) == 0);
	ASSERT(is_equal, "wrong data received from the server");
	len = ff_tcp_write_with_timeout(tcp_client, buf, 4, 100);
	ASSERT(len == 4, "written all data to the server");
	len = ff_tcp_flush_with_timeout(tcp_client, 100);
	ASSERT(len != -1, "data should be flushed");
	ff_tcp_delete(tcp_client);
	ff_tcp_delete(tcp_server);
	ff_arch_net_addr_delete(addr);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(tcp_all)
{
	RUN_TEST(tcp_create_delete);
	RUN_TEST(tcp_basic);
	return NULL;
}

/* end of ff_tcp tests */
#pragma endregion

#pragma region ff_udp tests

DECLARE_TEST(udp_create_delete)
{
	struct ff_udp *udp;

	trace_enter();
	ff_core_initialize();

	udp = ff_udp_create(FF_UDP_BROADCAST);
	ASSERT(udp != NULL, "broadcast udp should be created");
	ff_udp_delete(udp);

	udp = ff_udp_create(FF_UDP_UNICAST);
	ASSERT(udp != NULL, "unicast udp should be created");
	ff_udp_delete(udp);

	ff_core_shutdown();
	return NULL;
}

#define alert(msg) fprintf(stderr, "%s", msg)

static void fiberpool_udp_func(void *ctx)
{
	struct ff_udp *udp_server;
	int len;
	uint8_t buf[10];
	struct ff_arch_net_addr *client_addr;

	udp_server = (struct ff_udp *) ctx;
	client_addr = ff_arch_net_addr_create();
	len = ff_udp_read(udp_server, client_addr, buf, 10);
	if (len == 4)
	{
		int is_equal;

		is_equal = (memcmp(buf, "test", 4) == 0);
		if (is_equal)
		{
			len = ff_udp_write(udp_server, client_addr, buf, 4);
			if (len == 4)
			{
alert("read 10 bytes...");
				len = ff_udp_read(udp_server, client_addr, buf, 10);
alert("done\n");
				if (len == 9)
				{
alert("write 9 bytes...");
					len = ff_udp_write(udp_server, client_addr, buf, 9);
alert("done\n");
					if (len == 9)
					{
						ff_arch_net_addr_delete(client_addr);
					}
				}
			}
		}
	}
}

DECLARE_TEST(udp_basic)
{
	struct ff_udp *udp_client, *udp_server;
	int is_success;
	int len;
	int is_equal;
	struct ff_arch_net_addr *server_addr, *client_addr, *net_mask;
	uint8_t buf[10];

	trace_enter();
	ff_core_initialize();
	server_addr = ff_arch_net_addr_create();
	client_addr = ff_arch_net_addr_create();
	net_mask = ff_arch_net_addr_create();
	is_success = ff_arch_net_addr_resolve(net_mask, L"255.255.192.0", 0);
	ASSERT(is_success, "network mask should be resolved");
	is_success = ff_arch_net_addr_resolve(server_addr, L"10.6.27.90", 5432);
	ASSERT(is_success, "localhost address should be resolved");

	udp_server = ff_udp_create(FF_UDP_UNICAST);
	is_success = ff_udp_bind(udp_server, server_addr);
	ff_core_fiberpool_execute_async(fiberpool_udp_func, udp_server);
	udp_client = ff_udp_create(FF_UDP_UNICAST);

	len = ff_udp_write_with_timeout(udp_client, server_addr, "test", 4, 100);
	ASSERT(len == 4, "data should be sent to server");

	len = ff_udp_read(udp_client, client_addr, buf, 10);
	is_equal = ff_arch_net_addr_is_equal(server_addr, client_addr);
	ASSERT(is_equal, "server address should be eqaul to the client address");
	ASSERT(len == 4, "data should be read from the server");
	is_equal = (memcmp(buf, "test", 4) == 0);
	ASSERT(is_equal, "wrong data received from the server");

	len = ff_udp_read_with_timeout(udp_client, client_addr, buf, 10, 100);
	ASSERT(len == -1, "server shouldn't send data to client");
	ff_udp_delete(udp_client);

	udp_client = ff_udp_create(FF_UDP_BROADCAST);
	ff_arch_net_addr_get_broadcast_addr(server_addr, net_mask, server_addr);
	len = ff_udp_write(udp_client, server_addr, "broadcast", 9);
	ASSERT(len == 9, "broadcast data should be sent");
alert("reading response from broadcast...");
	len = ff_udp_read(udp_client, client_addr, buf, 10);
	ASSERT(len == 9, "server should send response with the given length");
alert("response received\n");
	is_equal = (memcmp(buf, "broadcast", 9) == 0);
	ASSERT(is_equal, "server sent wrong data");
	ff_udp_delete(udp_client);

	ff_udp_delete(udp_server);
	ff_arch_net_addr_delete(net_mask);
	ff_arch_net_addr_delete(client_addr);
	ff_arch_net_addr_delete(server_addr);
	ff_core_shutdown();
	return NULL;
}

DECLARE_TEST(udp_all)
{
	RUN_TEST(udp_create_delete);
	RUN_TEST(udp_basic);
	return NULL;
}

/* end of ff_udp tests */
#pragma endregion

static const char *run_all_tests()
{
/*	RUN_TEST(core_all);
	RUN_TEST(fiber_all);
	RUN_TEST(event_all);
	RUN_TEST(mutex_all);
	RUN_TEST(semaphore_all);
	RUN_TEST(blocking_queue_all);
	RUN_TEST(blocking_stack_all);
	RUN_TEST(pool_all);
	RUN_TEST(file_all);
	RUN_TEST(arch_net_addr_all);
	RUN_TEST(tcp_all);
	*/
	RUN_TEST(udp_all);
	return NULL;
}

int main(int argc, char* argv[])
{
	const char *msg;

	msg = run_all_tests();
	if (msg != NULL)
	{
		printf("%s\n", msg);
	}
	else
	{
		printf("ALL TESTS PASSED\n");
	}

	return 0;
}
