#include "ff/ff_common.h"
#include "ff/ff_core.h"
#include "ff/ff_log.h"
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
#include "ff/ff_stream_tcp.h"
#include "ff/ff_stream_tcp_with_timeout.h"
#include "ff/ff_udp.h"

#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
	#undef NDEBUG
	#include <assert.h>
	#define NDEBUG
#else
	#include <assert.h>
#endif

#define LOG_FILENAME L"ff_unit_tests_log.txt"

#define ASSERT(expr, msg) assert((expr) && (msg))

#pragma region ff_core_tests

static void test_core_init()
{
	ff_core_initialize(LOG_FILENAME);
	ff_core_shutdown();
}

static void test_core_init_multiple()
{
	int i;
	for (i = 0; i < 10; i++)
	{
		ff_core_initialize(LOG_FILENAME);
		ff_core_shutdown();
	}
}

static void test_core_sleep()
{
	ff_core_initialize(LOG_FILENAME);
	ff_core_sleep(100);
	ff_core_shutdown();
}

static void test_core_sleep_multiple()
{
	int i;

	ff_core_initialize(LOG_FILENAME);
	for (i = 0; i < 10; i++)
	{
		ff_core_sleep(i * 10 + 1);
	}
	ff_core_shutdown();
}

static void threadpool_int_increment(void *ctx)
{
	int *a;

	a = (int *) ctx;
	a[1] = a[0] + 1;
}

static void test_core_threadpool_execute()
{
	int a[2];

	ff_core_initialize(LOG_FILENAME);
	a[0] = 1234;
	a[1] = 4321;
	ff_core_threadpool_execute(threadpool_int_increment, a);
	ASSERT(a[1] == a[0] + 1, "unexpected result");
	ff_core_shutdown();
}

static void test_core_threadpool_execute_multiple()
{
	int i;

	ff_core_initialize(LOG_FILENAME);
	for (i = 0; i < 10; i++)
	{
		int a[2];

		a[0] = i;
		a[1] = i + 5;
		ff_core_threadpool_execute(threadpool_int_increment, a);
		ASSERT(a[1] == a[0] + 1, "unexpected result");
	}
	ff_core_shutdown();
}

static void fiberpool_int_increment(void *ctx)
{
	int *a;

	a = (int *) ctx;
	(*a)++;
}

static void test_core_fiberpool_execute()
{
	int a = 0;

	ff_core_initialize(LOG_FILENAME);
	ff_core_fiberpool_execute_async(fiberpool_int_increment, &a);
	ff_core_shutdown();
	ASSERT(a == 1, "unexpected result");
}

static void test_core_fiberpool_execute_multiple()
{
	int a = 0;
	int i;

	ff_core_initialize(LOG_FILENAME);
	for (i = 0; i < 10; i++)
	{
		ff_core_fiberpool_execute_async(fiberpool_int_increment, &a);
	}
	ff_core_shutdown();
	ASSERT(a == 10, "unexpected result");
}

static void test_core_fiberpool_execute_deferred()
{
	int a = 0;

	ff_core_initialize(LOG_FILENAME);
	ff_core_fiberpool_execute_deferred(fiberpool_int_increment, &a, 500);
	ff_core_sleep(10);
	ASSERT(a == 0, "unexpected result");
	ff_core_shutdown();
	ASSERT(a == 1, "unexpected result");
}

static void test_core_fiberpool_execute_deferred_multiple()
{
	int a = 0;
	int i;

	ff_core_initialize(LOG_FILENAME);
	for (i = 0; i < 10; i++)
	{
		ff_core_fiberpool_execute_deferred(fiberpool_int_increment, &a, 500);
	}
	ff_core_sleep(10);
	ASSERT(a == 0, "unexpected result");
	ff_core_shutdown();
	ASSERT(a == 10, "unexpected result");
}

static void test_core_all()
{
	test_core_init();
	test_core_init_multiple();
	test_core_sleep();
	test_core_sleep_multiple();
	test_core_threadpool_execute();
	test_core_threadpool_execute_multiple();
	test_core_fiberpool_execute();
	test_core_fiberpool_execute_multiple();
	test_core_fiberpool_execute_deferred();
	test_core_fiberpool_execute_deferred_multiple();
}

/* end of ff_core tests */
#pragma endregion

#pragma region ff_log tests

static void test_log_basic()
{
	ff_core_initialize(LOG_FILENAME);
	ff_log_info(L"this is an info log");
	ff_log_warning(L"this is a warning log");
	ff_core_shutdown();
}

static void test_log_all()
{
	test_log_basic();
}

/* end of ff_log tests */
#pragma endregion

#pragma region ff_fiber tests

static void fiber_func(void *ctx)
{
	int *a;

	a = (int *) ctx;
	(*a)++;
}

static void test_fiber_create_delete()
{
	struct ff_fiber *fiber;

	ff_core_initialize(LOG_FILENAME);
	fiber = ff_fiber_create(fiber_func, 0);
	ASSERT(fiber != NULL, "unexpected result");
	ff_fiber_delete(fiber);
	ff_core_shutdown();
}

static void test_fiber_start_join()
{
	struct ff_fiber *fiber;
	int a = 0;

	ff_core_initialize(LOG_FILENAME);
	fiber = ff_fiber_create(fiber_func, 0x100000);
	ff_fiber_start(fiber, &a);
	ff_fiber_join(fiber);
	ASSERT(a == 1, "unexpected result");
	ff_fiber_delete(fiber);
	ff_core_shutdown();
}

static void test_fiber_start_multiple()
{
	struct ff_fiber *fibers[10];
	int a = 0;
	int i;

	ff_core_initialize(LOG_FILENAME);
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
}

static void test_fiber_all()
{
	test_fiber_create_delete();
	test_fiber_start_join();
	test_fiber_start_multiple();
}

/* end of ff_fiber tests */
#pragma endregion

#pragma region ff_event tests

static void test_event_manual_create_delete()
{
	struct ff_event *event;

	ff_core_initialize(LOG_FILENAME);

	event = ff_event_create(FF_EVENT_MANUAL);
	ASSERT(event != NULL, "unexpected result");
	ff_event_delete(event);

	ff_core_shutdown();
}

static void test_event_auto_create_delete()
{
	struct ff_event *event;

	ff_core_initialize(LOG_FILENAME);

	event = ff_event_create(FF_EVENT_AUTO);
	ASSERT(event != NULL, "unexpected result");
	ff_event_delete(event);

	ff_core_shutdown();
}

static void fiberpool_event_setter(void *ctx)
{
	struct ff_event *event;

	event = (struct ff_event *) ctx;
	ff_event_set(event);
}

static void test_event_manual_basic()
{
	struct ff_event *event;
	int is_set;

	ff_core_initialize(LOG_FILENAME);
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
}

static void test_event_auto_basic()
{
	struct ff_event *event;
	int is_set;

	ff_core_initialize(LOG_FILENAME);
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
}

static void test_event_manual_timeout()
{
	struct ff_event *event;
	enum ff_result result;
	int is_set;

	ff_core_initialize(LOG_FILENAME);

	event = ff_event_create(FF_EVENT_MANUAL);
	result = ff_event_wait_with_timeout(event, 100);
	ASSERT(result != FF_SUCCESS, "event should timeout");
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "event should remain 'not set' after timeout");
	ff_event_set(event);
	result = ff_event_wait_with_timeout(event, 100);
	ASSERT(result == FF_SUCCESS, "event shouldn't timeout");
	is_set = ff_event_is_set(event);
	ASSERT(is_set, "manual event should remain set after ff_event_wait_with_timeout");
	ff_event_delete(event);

	ff_core_shutdown();
}

static void test_event_auto_timeout()
{
	struct ff_event *event;
	enum ff_result result;
	int is_set;

	ff_core_initialize(LOG_FILENAME);

	event = ff_event_create(FF_EVENT_AUTO);
	result = ff_event_wait_with_timeout(event, 100);
	ASSERT(result != FF_SUCCESS, "event should timeout");
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "event should remain 'not set' after timeout");
	ff_event_set(event);
	result = ff_event_wait_with_timeout(event, 100);
	ASSERT(result == FF_SUCCESS, "event shouldn't timeout");
	is_set = ff_event_is_set(event);
	ASSERT(!is_set, "auto reset event should be 'not set' after ff_event_wait_with_timeout");
	ff_event_delete(event);

	ff_core_shutdown();
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

static void test_event_manual_multiple()
{
	struct ff_event *event, *done_event;
	int i;
	int a = 0;
	void *data[3];

	ff_core_initialize(LOG_FILENAME);
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
}

static void test_event_auto_multiple()
{
	struct ff_event *event, *done_event;
	int i;
	int a = 0;
	void *data[3];

	ff_core_initialize(LOG_FILENAME);
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
		enum ff_result result;

		ff_event_set(event);
		result = ff_event_wait_with_timeout(done_event, 1);
		ASSERT(result != FF_SUCCESS, "done_event should remain 'not set'");
	}
	ff_event_set(event);
	ff_event_wait(done_event);
	ASSERT(a == 15, "a should have value 15 after done_event set");
	ff_event_delete(done_event);
	ff_event_delete(event);
	ff_core_shutdown();
}

static void test_event_all()
{
	test_event_manual_create_delete();
	test_event_auto_create_delete();
	test_event_manual_basic();
	test_event_auto_basic();
	test_event_manual_timeout();
	test_event_auto_timeout();
	test_event_manual_multiple();
	test_event_auto_multiple();
}

/* end of ff_event tests */
#pragma endregion

#pragma region ff_mutex tests

static void test_mutex_create_delete()
{
	struct ff_mutex *mutex;

	ff_core_initialize(LOG_FILENAME);
	mutex = ff_mutex_create();
	ASSERT(mutex != NULL, "mutex should be initialized");
	ff_mutex_delete(mutex);
	ff_core_shutdown();
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

static void test_mutex_basic()
{
	void *data[3];
	int a = 0;
	struct ff_mutex *mutex;
	struct ff_event *event;

	ff_core_initialize(LOG_FILENAME);
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
}

static void test_mutex_all()
{
	test_mutex_create_delete();
	test_mutex_basic();
}

/* end of ff_mutex tests */
#pragma endregion

#pragma region ff_semaphore tests

static void test_semaphore_create_delete()
{
	struct ff_semaphore *semaphore;

	ff_core_initialize(LOG_FILENAME);
	semaphore = ff_semaphore_create(0);
	ASSERT(semaphore != NULL, "semaphore should be initialized");
	ff_semaphore_delete(semaphore);
	ff_core_shutdown();
}

static void test_semaphore_basic()
{
	int i;
	enum ff_result result;
	struct ff_semaphore *semaphore;

	ff_core_initialize(LOG_FILENAME);
	semaphore = ff_semaphore_create(0);
	result = ff_semaphore_down_with_timeout(semaphore, 1);
	ASSERT(result != FF_SUCCESS, "semaphore with 0 value cannot be down");
	for (i = 0; i < 10; i++)
	{
		ff_semaphore_up(semaphore);
	}
	result = ff_semaphore_down_with_timeout(semaphore, 1);
	ASSERT(result == FF_SUCCESS, "semaphore should be down");
	for (i = 0; i < 9; i++)
	{
		ff_semaphore_down(semaphore);
	}
	result = ff_semaphore_down_with_timeout(semaphore, 1);
	ASSERT(result != FF_SUCCESS, "semaphore cannot be down");
	ff_semaphore_delete(semaphore);
	ff_core_shutdown();
}

static void test_semaphore_all()
{
	test_semaphore_create_delete();
	test_semaphore_basic();
}

/* end of ff_semaphore tests */
#pragma endregion

#pragma region ff_blocking_queue tests

static void test_blocking_queue_create_delete()
{
	struct ff_blocking_queue *queue;

	ff_core_initialize(LOG_FILENAME);
	queue = ff_blocking_queue_create(10);
	ASSERT(queue != NULL, "queue should be initialized");
	ff_blocking_queue_delete(queue);
	ff_core_shutdown();
}

static void test_blocking_queue_basic()
{
	int64_t i;
	enum ff_result result;
	int64_t data = 0;
	struct ff_blocking_queue *queue;
	int is_empty;

	ff_core_initialize(LOG_FILENAME);
	queue = ff_blocking_queue_create(10);
	is_empty = ff_blocking_queue_is_empty(queue);
	ASSERT(is_empty, "queue should be empty");
	for (i = 0; i < 10; i++)
	{
		ff_blocking_queue_put(queue, *(void **)&i);
	}
	is_empty = ff_blocking_queue_is_empty(queue);
	ASSERT(!is_empty, "queue shouldn't be empty");
	result = ff_blocking_queue_put_with_timeout(queue, (void *)123, 1);
	ASSERT(result != FF_SUCCESS, "queue should be full");
	for (i = 0; i < 10; i++)
	{
		ff_blocking_queue_get(queue, (const void **)&data);
		ASSERT(data == i, "wrong value received from the queue");
	}
	is_empty = ff_blocking_queue_is_empty(queue);
	ASSERT(is_empty, "queue should be empty");
	result = ff_blocking_queue_get_with_timeout(queue, (const void **)&data, 1);
	ASSERT(result != FF_SUCCESS, "queue should be empty");
	ff_blocking_queue_delete(queue);
	ff_core_shutdown();
}

static void fiberpool_blocking_queue_func(void *ctx)
{
	struct ff_blocking_queue *queue;

	queue = (struct ff_blocking_queue *) ctx;
	ff_blocking_queue_put(queue, (void *)543);
}

static void test_blocking_queue_fiberpool()
{
	int64_t data = 0;
	enum ff_result result;
	struct ff_blocking_queue *queue;

	ff_core_initialize(LOG_FILENAME);
	queue = ff_blocking_queue_create(1);
	result = ff_blocking_queue_get_with_timeout(queue, (const void **)&data, 1);
	ASSERT(result != FF_SUCCESS, "queue should be empty");
	ff_core_fiberpool_execute_async(fiberpool_blocking_queue_func, queue);
	ff_blocking_queue_get(queue, (const void **)&data);
	ASSERT(data == 543, "unexpected value received from the queue");
	result = ff_blocking_queue_get_with_timeout(queue, (const void **)&data, 1);
	ASSERT(result != FF_SUCCESS, "queue shouldn't have values");
	ff_blocking_queue_delete(queue);
	ff_core_shutdown();
}

static void test_blocking_queue_all()
{
	test_blocking_queue_create_delete();
	test_blocking_queue_basic();
	test_blocking_queue_fiberpool();
}

/* end of ff_blocking_queue tests */
#pragma endregion

#pragma region ff_blocking_stack tests

static void test_blocking_stack_create_delete()
{
	struct ff_blocking_stack *stack;

	ff_core_initialize(LOG_FILENAME);
	stack = ff_blocking_stack_create(10);
	ASSERT(stack != NULL, "stack should be initialized");
	ff_blocking_stack_delete(stack);
	ff_core_shutdown();
}

static void test_blocking_stack_basic()
{
	int64_t i;
	enum ff_result result;
	int64_t data = 0;
	struct ff_blocking_stack *stack;

	ff_core_initialize(LOG_FILENAME);
	stack = ff_blocking_stack_create(10);
	for (i = 0; i < 10; i++)
	{
		ff_blocking_stack_push(stack, *(void **)&i);
	}
	result = ff_blocking_stack_push_with_timeout(stack, (void *)1234, 1);
	ASSERT(result != FF_SUCCESS, "stack should be fulll");
	for (i = 9; i >= 0; i--)
	{
		ff_blocking_stack_pop(stack, (const void **)&data);
		ASSERT(data == i, "wrong value retrieved from the stack");
	}
	result = ff_blocking_stack_pop_with_timeout(stack, (const void **)&data, 1);
	ASSERT(result != FF_SUCCESS, "stack should be empty");
	ff_blocking_stack_delete(stack);
	ff_core_shutdown();
}

static void fiberpool_blocking_stack_func(void *ctx)
{
	struct ff_blocking_stack *stack;

	stack = (struct ff_blocking_stack *) ctx;
	ff_blocking_stack_push(stack, (void *)543);
}

static void test_blocking_stack_fiberpool()
{
	int64_t data = 0;
	enum ff_result result;
	struct ff_blocking_stack *stack;

	ff_core_initialize(LOG_FILENAME);
	stack = ff_blocking_stack_create(1);
	result = ff_blocking_stack_pop_with_timeout(stack, (const void **)&data, 1);
	ASSERT(result != FF_SUCCESS, "stack should be empty");
	ff_core_fiberpool_execute_async(fiberpool_blocking_stack_func, stack);
	ff_blocking_stack_pop(stack, (const void **)&data);
	ASSERT(data == 543, "unexpected value received from the stack");
	result = ff_blocking_stack_pop_with_timeout(stack, (const void **)&data, 1);
	ASSERT(result != FF_SUCCESS, "stack shouldn't have values");
	ff_blocking_stack_delete(stack);
	ff_core_shutdown();
}

static void test_blocking_stack_all()
{
	test_blocking_stack_create_delete();
	test_blocking_stack_basic();
	test_blocking_stack_fiberpool();
}

/* end of ff_blocking_stack tests */
#pragma endregion

#pragma region ff_pool tests

static int pool_entries_cnt = 0;

static void *pool_entry_constructor(void *ctx)
{
	ASSERT(pool_entries_cnt >= 0, "unexpected pool entries value");
	pool_entries_cnt++;
	return (void *)123;
}

static void pool_entry_destructor(void *entry)
{
	ASSERT(pool_entries_cnt > 0, "unexpected pool entries value");
	pool_entries_cnt--;
}

static void test_pool_create_delete()
{
	struct ff_pool *pool;

	ff_core_initialize(LOG_FILENAME);
	pool = ff_pool_create(10, pool_entry_constructor, NULL, pool_entry_destructor);
	ASSERT(pool_entries_cnt == 0, "pool should be empty after creation");
	ff_pool_delete(pool);
	ASSERT(pool_entries_cnt == 0, "pool should be empty after deletion");
	ff_core_shutdown();
}

static void pool_visitor_func(void *entry, void *ctx, int is_acquired)
{
	int *a;

	a = (int *) ctx;
	if (is_acquired)
	{
		(*a)++;
	}
}

static void test_pool_basic()
{
	struct ff_pool *pool;
	void *entry;
	int i;
	int a;

	ff_core_initialize(LOG_FILENAME);
	pool = ff_pool_create(10, pool_entry_constructor, NULL, pool_entry_destructor);
	for (i = 0; i < 10; i++)
	{
		entry = ff_pool_acquire_entry(pool);
		ASSERT(entry == (void *)123, "unexpected value for the entry");
		ASSERT(pool_entries_cnt == i + 1, "unexpected entries number");
	}

	a = 0;
	ff_pool_for_each_entry(pool, pool_visitor_func, &a);
	ASSERT(a == 10, "unexpected result");

	for (i = 0; i < 10; i++)
	{
		ff_pool_release_entry(pool, (void *)123);
	}

	a = 0;
	ff_pool_for_each_entry(pool, pool_visitor_func, &a);
	ASSERT(a == 0, "unexpected result");

	ff_pool_delete(pool);
	ASSERT(pool_entries_cnt == 0, "pool should be empty after deletion");
	ff_core_shutdown();
}

static void fiberpool_pool_func(void *ctx)
{
	struct ff_pool *pool;

	pool = (struct ff_pool *) ctx;
	ff_pool_release_entry(pool, (void *)123);
}

static void test_pool_fiberpool()
{
	struct ff_pool *pool;
	void *entry;
	
	ff_core_initialize(LOG_FILENAME);
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
}

static void test_pool_all()
{
	test_pool_create_delete();
	test_pool_basic();
	test_pool_fiberpool();
}

/* end of ff_pool tests */
#pragma endregion

#pragma region ff_file tests

static void test_file_open_read_fail()
{
	struct ff_file *file;

	ff_core_initialize(LOG_FILENAME);
	file = ff_file_open(L"unexpected_file.txt", FF_FILE_READ);
	ASSERT(file == NULL, "unexpected file couldn't be opened");
	ff_core_shutdown();
}

static void test_file_create_delete()
{
	struct ff_file *file;
	enum ff_result result;

	ff_core_initialize(LOG_FILENAME);

	file = ff_file_open(L"test.txt", FF_FILE_WRITE);
	ASSERT(file != NULL, "file should be created");
	ff_file_close(file);
	result = ff_file_erase(L"test.txt");
	ASSERT(result == FF_SUCCESS, "file should be deleted");
	result = ff_file_erase(L"test.txt");
	ASSERT(result != FF_SUCCESS, "file already deleted");

	ff_core_shutdown();
}

static void test_file_basic()
{
	struct ff_file *file;
	int64_t size;
	uint8_t data[] = "hello, world!\n";
	uint8_t buf[sizeof(data) - 1];
	int is_equal;
	enum ff_result result;

	ff_core_initialize(LOG_FILENAME);

	file = ff_file_open(L"test.txt", FF_FILE_WRITE);
	ASSERT(file != NULL, "file should be created");
	size = ff_file_get_size(file);
	ASSERT(size == 0, "file should be empty");
	result = ff_file_write(file, data, sizeof(data) - 1);
	ASSERT(result == FF_SUCCESS, "all the data should be written to the file");
	result = ff_file_flush(file);
	ASSERT(result == FF_SUCCESS, "file should be flushed successfully");
	ff_file_close(file);

	file = ff_file_open(L"test.txt", FF_FILE_READ);
	ASSERT(file != NULL, "file should exist");
	size = ff_file_get_size(file);
	ASSERT(size == sizeof(data) - 1, "wrong file size");
	result = ff_file_read(file, buf, sizeof(data) - 1);
	ASSERT(result == FF_SUCCESS, "unexpected length of data read from the file");
	is_equal = (memcmp(data, buf, sizeof(data) - 1) == 0);
	ASSERT(is_equal, "wrong data read from the file");
	ff_file_close(file);

	result = ff_file_copy(L"test.txt", L"test1.txt");
	ASSERT(result == FF_SUCCESS, "file should be copied");
	result = ff_file_copy(L"test.txt", L"test1.txt");
	ASSERT(result != FF_SUCCESS, "cannot copy to existing file");
	result = ff_file_move(L"test.txt", L"test2.txt");
	ASSERT(result == FF_SUCCESS, "file should be moved");
	result = ff_file_move(L"test.txt", L"test2.txt");
	ASSERT(result != FF_SUCCESS, "cannot move to existing file");

	file = ff_file_open(L"test1.txt", FF_FILE_READ);
	ASSERT(file != NULL, "file copy should exist");
	result = ff_file_read(file, buf, sizeof(data) - 1);
	ASSERT(result == FF_SUCCESS, "unexpected length of file copy");
	is_equal = (memcmp(data, buf, sizeof(data) - 1) == 0);
	ASSERT(is_equal, "wrong contents of the file copy");
	ff_file_close(file);

	file = ff_file_open(L"test1.txt", FF_FILE_WRITE);
	ASSERT(file != NULL, "file should be truncated");
	size = ff_file_get_size(file);
	ASSERT(size == 0, "truncated file should be empty");
	ff_file_close(file);

	result = ff_file_erase(L"test1.txt");
	ASSERT(result == FF_SUCCESS, "file1 should be deleted");
	result = ff_file_erase(L"test2.txt");
	ASSERT(result == FF_SUCCESS, "file2 should be deleted");

	ff_core_shutdown();
}

static void test_file_all()
{
	test_file_open_read_fail();
	test_file_create_delete();
	test_file_basic();
}

/* end of ff_file tests */
#pragma endregion

#pragma region ff_arch_net_addr tests

static void test_arch_net_addr_create_delete()
{
	struct ff_arch_net_addr *addr;

	ff_core_initialize(LOG_FILENAME);
	addr = ff_arch_net_addr_create();
	ASSERT(addr != NULL, "addr should be created");
	ff_arch_net_addr_delete(addr);
	ff_core_shutdown();
}

static void test_arch_net_addr_resolve_success()
{
	struct ff_arch_net_addr *addr1, *addr2;
	enum ff_result result;
	int is_equal;

	ff_core_initialize(LOG_FILENAME);
	addr1 = ff_arch_net_addr_create();
	addr2 = ff_arch_net_addr_create();
	result = ff_arch_net_addr_resolve(addr1, L"localhost", 80);
	ASSERT(result == FF_SUCCESS, "localhost address should be resolved successfully");
	result = ff_arch_net_addr_resolve(addr2, L"127.0.0.1", 80);
	ASSERT(result == FF_SUCCESS, "127.0.0.1 address should be resolved successfully");
	is_equal = ff_arch_net_addr_is_equal(addr1, addr2);
	ASSERT(is_equal, "addresses should be equal");
	result = ff_arch_net_addr_resolve(addr2, L"123.45.1.1", 0);
	ASSERT(result == FF_SUCCESS, "numeric address should be resolved successfully");
	is_equal = ff_arch_net_addr_is_equal(addr1, addr2);
	ASSERT(!is_equal, "addresses shouldn't be equivalent");
	ff_arch_net_addr_delete(addr1);
	ff_arch_net_addr_delete(addr2);
	ff_core_shutdown();
}

static void test_arch_net_addr_resolve_fail()
{
	struct ff_arch_net_addr *addr;
	enum ff_result result;

	ff_core_initialize(LOG_FILENAME);
	addr = ff_arch_net_addr_create();
	result = ff_arch_net_addr_resolve(addr, L"non.existant,address", 123);
	ASSERT(result != FF_SUCCESS, "address shouldn't be resolved");
	ff_arch_net_addr_delete(addr);
	ff_core_shutdown();
}

static void test_arch_net_addr_broadcast()
{
	struct ff_arch_net_addr *addr, *net_mask, *broadcast_addr;
	enum ff_result result;
	int is_equal;

	ff_core_initialize(LOG_FILENAME);
	addr = ff_arch_net_addr_create();
	net_mask = ff_arch_net_addr_create();
	broadcast_addr = ff_arch_net_addr_create();

	result = ff_arch_net_addr_resolve(addr, L"123.45.67.89", 123);
	ASSERT(result == FF_SUCCESS, "address should be resolved");
	result = ff_arch_net_addr_resolve(net_mask, L"255.255.0.0", 0);
	ASSERT(result == FF_SUCCESS, "net mask should be resolved");
	ff_arch_net_addr_get_broadcast_addr(addr, net_mask, broadcast_addr);
	result = ff_arch_net_addr_resolve(addr, L"123.45.255.255", 123);
	ASSERT(result == FF_SUCCESS, "broadcast address should be resolved");
	is_equal = ff_arch_net_addr_is_equal(addr, broadcast_addr);
	ASSERT(is_equal, "addresses should be equal");

	ff_arch_net_addr_delete(addr);
	ff_arch_net_addr_delete(net_mask);
	ff_arch_net_addr_delete(broadcast_addr);
	ff_core_shutdown();
}

static void test_arch_net_addr_to_string()
{
	struct ff_arch_net_addr *addr;
	const wchar_t *str;
	enum ff_result result;
	int is_equal;

	ff_core_initialize(LOG_FILENAME);
	addr = ff_arch_net_addr_create();
	result = ff_arch_net_addr_resolve(addr, L"12.34.56.78", 90);
	ASSERT(result == FF_SUCCESS, "address should be resolved");
	str = ff_arch_net_addr_to_string(addr);
	is_equal = (wcscmp(L"12.34.56.78:90", str) == 0);
	ASSERT(is_equal, "wrong string");
	ff_arch_net_addr_delete_string(str);
	ff_arch_net_addr_delete(addr);
	ff_core_shutdown();
}

static void test_arch_net_addr_all()
{
	test_arch_net_addr_create_delete();
	test_arch_net_addr_resolve_success();
	test_arch_net_addr_resolve_fail();
	test_arch_net_addr_broadcast();
	test_arch_net_addr_to_string();
}

/* end of ff_arch_net_addr tests */
#pragma endregion

#pragma region ff_tcp tests

static void test_tcp_create_delete()
{
	struct ff_tcp *tcp;

	ff_core_initialize(LOG_FILENAME);
	tcp = ff_tcp_create();
	ASSERT(tcp != NULL, "tcp should be created");
	ff_tcp_delete(tcp);
	ff_core_shutdown();
}

static void fiberpool_tcp_func(void *ctx)
{
	struct ff_tcp *tcp_server, *tcp_client;
	struct ff_arch_net_addr *client_addr;
	uint8_t buf[4];
	int is_equal;
	enum ff_result result;

	tcp_server = (struct ff_tcp *) ctx;
	client_addr = ff_arch_net_addr_create();
	tcp_client = ff_tcp_accept(tcp_server, client_addr);
	ASSERT(tcp_client != NULL, "ff_tcp_accept() should return valid tcp_client");
	result = ff_tcp_write(tcp_client, "test", 4);
	ASSERT(result == FF_SUCCESS, "cannot write data to the tcp");
	result = ff_tcp_flush(tcp_client);
	ASSERT(result == FF_SUCCESS, "ff_tcp_flush() returned unexpected value");
	result = ff_tcp_read(tcp_client, buf, 4);
	ASSERT(result == FF_SUCCESS, "ff_tcp_read() failed");
	is_equal = (memcmp("test", buf, 4) == 0);
	ASSERT(is_equal, "wrong data received from the client");
	ff_tcp_delete(tcp_client);
	ff_arch_net_addr_delete(client_addr);
}

static void test_tcp_basic()
{
	struct ff_tcp *tcp_server, *tcp_client;
	struct ff_arch_net_addr *addr;
	enum ff_result result;
	int is_equal;
	uint8_t buf[4];

	ff_core_initialize(LOG_FILENAME);
	addr = ff_arch_net_addr_create();
	result = ff_arch_net_addr_resolve(addr, L"127.0.0.1", 43210);
	ASSERT(result == FF_SUCCESS, "localhost address should be resolved successfully");
	tcp_server = ff_tcp_create();
	ASSERT(tcp_server != NULL, "server should be created");
	result = ff_tcp_bind(tcp_server, addr, FF_TCP_SERVER);
	ASSERT(result == FF_SUCCESS, "server should be bound to local address");
	ff_core_fiberpool_execute_async(fiberpool_tcp_func, tcp_server);

	tcp_client = ff_tcp_create();
	ASSERT(tcp_client != NULL, "client should be created");
	result = ff_tcp_connect(tcp_client, addr);
	ASSERT(result == FF_SUCCESS, "client should connect to the server");
	result = ff_tcp_read_with_timeout(tcp_client, buf, 4, 100000);
	ASSERT(result == FF_SUCCESS, "unexpected data received from the server");
	is_equal = (memcmp(buf, "test", 4) == 0);
	ASSERT(is_equal, "wrong data received from the server");
	result = ff_tcp_write_with_timeout(tcp_client, buf, 4, 100);
	ASSERT(result == FF_SUCCESS, "written all data to the server");
	result = ff_tcp_flush_with_timeout(tcp_client, 100);
	ASSERT(result == FF_SUCCESS, "data should be flushed");
	ff_tcp_delete(tcp_client);
	ff_tcp_delete(tcp_server);
	ff_arch_net_addr_delete(addr);
	ff_core_shutdown();
}

struct tcp_server_shutdown_data
{
	struct ff_tcp *tcp_server1;
	struct ff_tcp *tcp_server2;
	struct ff_event *event;
};

static void fiberpool_tcp_server_shutdown_func(void *ctx)
{
	struct tcp_server_shutdown_data *data;
	struct ff_arch_net_addr *client_addr;
	struct ff_tcp *tcp_client;

	data = (struct tcp_server_shutdown_data *) ctx;
	client_addr = ff_arch_net_addr_create();
	tcp_client = ff_tcp_accept(data->tcp_server1, client_addr);
	ASSERT(tcp_client == NULL, "tcp_server1 should be already shutdowned, so ff_tcp_accept() should return NULL");
	ff_event_set(data->event);
	tcp_client = ff_tcp_accept(data->tcp_server2, client_addr);
	ASSERT(tcp_client == NULL, "tcp_server2 should be shutdowned when ff_tcp_accept() was blocking, so it should return NULL");
	ff_event_set(data->event);
	ff_arch_net_addr_delete(client_addr);
}

static void test_tcp_server_shutdown()
{
	struct ff_arch_net_addr *addr1, *addr2;
	struct tcp_server_shutdown_data data;
	enum ff_result result;

	ff_core_initialize(LOG_FILENAME);
	addr1 = ff_arch_net_addr_create();
	addr2 = ff_arch_net_addr_create();
	result = ff_arch_net_addr_resolve(addr1, L"127.0.0.1", 43211);
	ASSERT(result == FF_SUCCESS, "localhost address should be resolved successfully");
	result = ff_arch_net_addr_resolve(addr2, L"127.0.0.1", 43212);
	ASSERT(result == FF_SUCCESS, "localhost address should be resolved successfully");
	data.tcp_server1 = ff_tcp_create();
	data.tcp_server2 = ff_tcp_create();
	result = ff_tcp_bind(data.tcp_server1, addr1, FF_TCP_SERVER);
	ASSERT(result == FF_SUCCESS, "server should be bound to local address");
	result = ff_tcp_bind(data.tcp_server2, addr2, FF_TCP_SERVER);
	ASSERT(result == FF_SUCCESS, "cannot bind server to local address");
	data.event = ff_event_create(FF_EVENT_AUTO);
	ff_core_fiberpool_execute_async(fiberpool_tcp_server_shutdown_func, &data);
	ff_tcp_disconnect(data.tcp_server1);
	ff_event_wait(data.event);
	ff_tcp_disconnect(data.tcp_server2);
	ff_event_wait(data.event);

	ff_event_delete(data.event);
	ff_tcp_delete(data.tcp_server2);
	ff_tcp_delete(data.tcp_server1);
	ff_arch_net_addr_delete(addr2);
	ff_arch_net_addr_delete(addr1);
	ff_core_shutdown();
}

static void test_tcp_all()
{
	test_tcp_create_delete();
	test_tcp_basic();
	test_tcp_server_shutdown();
}

/* end of ff_tcp tests */
#pragma endregion

#pragma region ff_stream_tcp tests

static void test_stream_tcp_create_delete()
{
	struct ff_tcp *tcp;
	struct ff_stream *stream;

	ff_core_initialize(LOG_FILENAME);
	tcp = ff_tcp_create();
	stream = ff_stream_tcp_create(tcp);
	ff_stream_delete(stream);
	/* there is no need to call ff_tcp_delete() here,
	 * because ff_stream_delete() should already make this call
	 */
	ff_core_shutdown();
}

struct stream_tcp_basic_data
{
	struct ff_tcp *server_tcp;
	struct ff_event *event;
};

static void stream_tcp_basic_func(void *ctx)
{
	struct stream_tcp_basic_data *data;
	struct ff_tcp *client_tcp;
	struct ff_arch_net_addr *remote_addr;
	struct ff_stream *client_stream;
	uint8_t buf[3];
	enum ff_result result;
	int is_equal;

	data = (struct stream_tcp_basic_data *) ctx;
	remote_addr = ff_arch_net_addr_create();
	client_tcp = ff_tcp_accept(data->server_tcp, remote_addr);
	ASSERT(client_tcp != NULL, "cannot accept local TCP connection");
	client_stream = ff_stream_tcp_create(client_tcp);
	result = ff_stream_write(client_stream, "foo", 3);
	ASSERT(result == FF_SUCCESS, "error when writing to tcp stream");
	result = ff_stream_flush(client_stream);
	ASSERT(result == FF_SUCCESS, "error when flushing tcp stream");
	result = ff_stream_read(client_stream, buf, 3);
	ASSERT(result == FF_SUCCESS, "error when reading tcp stream");
	is_equal = (memcmp(buf, "bar", 3) == 0);
	ASSERT(is_equal, "wrong data received from tcp stream");
	ff_stream_disconnect(client_stream);
	result = ff_stream_read(client_stream, buf, 3);
	ASSERT(result != FF_SUCCESS, "stream didn't disconnected");
	result = ff_stream_write(client_stream, "foo", 3);
	ASSERT(result != FF_SUCCESS, "stream didn't disconnected");
	ff_stream_delete(client_stream);
	ff_event_wait(data->event);
	ff_arch_net_addr_delete(remote_addr);
}

static void test_stream_tcp_basic()
{
	struct stream_tcp_basic_data data;
	struct ff_arch_net_addr *addr;
	struct ff_tcp *client_tcp;
	struct ff_stream *client_stream;
	uint8_t buf[3];
	enum ff_result result;
	int is_equal;

	ff_core_initialize(LOG_FILENAME);
	data.server_tcp = ff_tcp_create();
	data.event = ff_event_create(FF_EVENT_AUTO);
	addr = ff_arch_net_addr_create();
	result = ff_arch_net_addr_resolve(addr, L"localhost", 8393);
	ASSERT(result == FF_SUCCESS, "cannot resolve localhost address");
	result = ff_tcp_bind(data.server_tcp, addr, FF_TCP_SERVER);
	ASSERT(result == FF_SUCCESS, "cannot bind server tcp");
	ff_core_fiberpool_execute_async(stream_tcp_basic_func, &data);
	client_tcp = ff_tcp_create();
	result = ff_tcp_connect(client_tcp, addr);
	ASSERT(result == FF_SUCCESS, "cannot connect to local tcp");
	client_stream = ff_stream_tcp_create(client_tcp);
	result = ff_stream_read(client_stream, buf, 3);
	ASSERT(result == FF_SUCCESS, "cannot read data from the stream");
	is_equal = (memcmp(buf, "foo", 3) == 0);
	ASSERT(is_equal, "unexpected data received from the stream");
	result = ff_stream_write(client_stream, "bar", 3);
	ASSERT(result == FF_SUCCESS, "cannot write data to the stream");
	result = ff_stream_flush(client_stream);
	ASSERT(result == FF_SUCCESS, "cannot flush the stream");
	result = ff_stream_read(client_stream, buf, 3);
	ASSERT(result != FF_SUCCESS, "stream should be disconnected");
	ff_stream_disconnect(client_stream);
	result = ff_stream_write(client_stream, "bar", 3);
	ASSERT(result != FF_SUCCESS, "stream should be disconnected");
	ff_event_set(data.event);
	ff_stream_delete(client_stream);
	ff_arch_net_addr_delete(addr);
	ff_event_delete(data.event);
	ff_tcp_delete(data.server_tcp);
	ff_core_shutdown();
}

static void test_stream_tcp_all()
{
	test_stream_tcp_create_delete();
	test_stream_tcp_basic();
}

/* end of ff_stream_tcp tests */
#pragma endregion

#pragma region ff_stream_tcp_with_timeout tests

static void test_stream_tcp_with_timeout_create_delete()
{
	struct ff_tcp *tcp;
	struct ff_stream *stream;

	ff_core_initialize(LOG_FILENAME);
	tcp = ff_tcp_create();
	stream = ff_stream_tcp_with_timeout_create(tcp, 1000, 1000);
	ff_stream_delete(stream);
	/* there is no need to call ff_tcp_delete() here,
	 * because ff_stream_delete() should already make this call
	 */
	ff_core_shutdown();
}

struct stream_tcp_with_timeout_basic_data
{
	struct ff_tcp *server_tcp;
	struct ff_event *event;
};

static void stream_tcp_with_timeout_basic_func(void *ctx)
{
	struct stream_tcp_with_timeout_basic_data *data;
	struct ff_tcp *client_tcp;
	struct ff_arch_net_addr *remote_addr;
	struct ff_stream *client_stream;
	uint8_t buf[3];
	enum ff_result result;
	int is_equal;

	data = (struct stream_tcp_with_timeout_basic_data *) ctx;
	remote_addr = ff_arch_net_addr_create();
	client_tcp = ff_tcp_accept(data->server_tcp, remote_addr);
	ASSERT(client_tcp != NULL, "cannot accept local TCP connection");
	client_stream = ff_stream_tcp_with_timeout_create(client_tcp, 500, 500);
	result = ff_stream_write(client_stream, "foo", 3);
	ASSERT(result == FF_SUCCESS, "error when writing to tcp stream");
	result = ff_stream_flush(client_stream);
	ASSERT(result == FF_SUCCESS, "error when flushing tcp stream");
	result = ff_stream_read(client_stream, buf, 3);
	ASSERT(result == FF_SUCCESS, "error when reading tcp stream");
	is_equal = (memcmp(buf, "bar", 3) == 0);
	ASSERT(is_equal, "wrong data received from tcp stream");
	result = ff_stream_read(client_stream, buf, 3);
	ASSERT(result != FF_SUCCESS, "stream shouldn't provide any data");
	ff_event_wait(data->event);
	ff_stream_delete(client_stream);
	ff_arch_net_addr_delete(remote_addr);
}

static void test_stream_tcp_with_timeout_basic()
{
	struct stream_tcp_with_timeout_basic_data data;
	struct ff_arch_net_addr *addr;
	struct ff_tcp *client_tcp;
	struct ff_stream *client_stream;
	uint8_t buf[3];
	enum ff_result result;
	int is_equal;

	ff_core_initialize(LOG_FILENAME);
	data.server_tcp = ff_tcp_create();
	data.event = ff_event_create(FF_EVENT_AUTO);
	addr = ff_arch_net_addr_create();
	result = ff_arch_net_addr_resolve(addr, L"localhost", 8394);
	ASSERT(result == FF_SUCCESS, "cannot resolve localhost address");
	result = ff_tcp_bind(data.server_tcp, addr, FF_TCP_SERVER);
	ASSERT(result == FF_SUCCESS, "cannot bind server tcp");
	ff_core_fiberpool_execute_async(stream_tcp_with_timeout_basic_func, &data);
	client_tcp = ff_tcp_create();
	result = ff_tcp_connect(client_tcp, addr);
	ASSERT(result == FF_SUCCESS, "cannot connect to local tcp");
	client_stream = ff_stream_tcp_with_timeout_create(client_tcp, 200, 200);
	result = ff_stream_read(client_stream, buf, 3);
	ASSERT(result == FF_SUCCESS, "cannot read data from the stream");
	is_equal = (memcmp(buf, "foo", 3) == 0);
	ASSERT(is_equal, "unexpected data received from the stream");
	result = ff_stream_write(client_stream, "bar", 3);
	ASSERT(result == FF_SUCCESS, "cannot write data to the stream");
	result = ff_stream_flush(client_stream);
	ASSERT(result == FF_SUCCESS, "cannot flush the stream");
	result = ff_stream_read(client_stream, buf, 3);
	ASSERT(result != FF_SUCCESS, "stream shouldn't provide any data");
	ff_event_set(data.event);
	ff_stream_delete(client_stream);
	ff_arch_net_addr_delete(addr);
	ff_event_delete(data.event);
	ff_tcp_delete(data.server_tcp);
	ff_core_shutdown();
}

static void test_stream_tcp_with_timeout_all()
{
	test_stream_tcp_with_timeout_create_delete();
	test_stream_tcp_with_timeout_basic();
}

/* end of ff_stream_tcp_with_timeout tests */
#pragma endregion

#pragma region ff_udp tests

static void test_udp_create_delete()
{
	struct ff_udp *udp;

	ff_core_initialize(LOG_FILENAME);

	udp = ff_udp_create(FF_UDP_BROADCAST);
	ASSERT(udp != NULL, "broadcast udp should be created");
	ff_udp_delete(udp);

	udp = ff_udp_create(FF_UDP_UNICAST);
	ASSERT(udp != NULL, "unicast udp should be created");
	ff_udp_delete(udp);

	ff_core_shutdown();
}

static void fiberpool_udp_func(void *ctx)
{
	struct ff_udp *udp_server;
	int len;
	uint8_t buf[10];
	struct ff_arch_net_addr *client_addr;
	int is_equal;

	udp_server = (struct ff_udp *) ctx;
	client_addr = ff_arch_net_addr_create();
	len = ff_udp_read(udp_server, client_addr, buf, 10);
	ASSERT(len == 4, "ff_udp_read() should read 4 bytes");
	is_equal = (memcmp(buf, "test", 4) == 0);
	ASSERT(is_equal, "ff_udp_read() should receive 'test' message");
	len = ff_udp_write(udp_server, client_addr, buf, 4);
	ASSERT(len == 4, "ff_udp_write() should write exactly 5 bytes");
	len = ff_udp_read(udp_server, client_addr, buf, 10);
	ASSERT(len == 9, "ff_udp_read() should read 9 bytes");
	is_equal = (memcmp(buf, "broadcast", 9) == 0);
	ASSERT(is_equal, "ff_udp_read() should receive 'broadcast' message");
	len = ff_udp_write(udp_server, client_addr, buf, 9);
	ASSERT(len == 9, "ff_udp_write() should write 9 bytes");
	ff_arch_net_addr_delete(client_addr);
}

static void test_udp_basic()
{
	struct ff_udp *udp_client, *udp_server;
	enum ff_result result;
	int len;
	int is_equal;
	struct ff_arch_net_addr *server_addr, *client_addr, *net_mask;
	uint8_t buf[10];

	ff_core_initialize(LOG_FILENAME);
	server_addr = ff_arch_net_addr_create();
	client_addr = ff_arch_net_addr_create();
	net_mask = ff_arch_net_addr_create();
	result = ff_arch_net_addr_resolve(net_mask, L"255.0.0.0", 0);
	ASSERT(result == FF_SUCCESS, "network mask should be resolved");
	result = ff_arch_net_addr_resolve(server_addr, L"127.0.0.1", 5432);
	ASSERT(result == FF_SUCCESS, "localhost address should be resolved");
	udp_server = ff_udp_create(FF_UDP_UNICAST);
	result = ff_udp_bind(udp_server, server_addr);
	ASSERT(result == FF_SUCCESS, "cannot bind local udp address");
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
	len = ff_udp_read(udp_client, client_addr, buf, 10);
	ASSERT(len == 9, "server should send response with the given length");
	is_equal = (memcmp(buf, "broadcast", 9) == 0);
	ASSERT(is_equal, "server sent wrong data");
	ff_udp_delete(udp_client);

	ff_udp_delete(udp_server);
	ff_arch_net_addr_delete(net_mask);
	ff_arch_net_addr_delete(client_addr);
	ff_arch_net_addr_delete(server_addr);
	ff_core_shutdown();
}

static void test_udp_all()
{
	test_udp_create_delete();
	test_udp_basic();
}

/* end of ff_udp tests */
#pragma endregion

static void test_all()
{
	test_core_all();
	test_log_all();
	test_fiber_all();
	test_event_all();
	test_mutex_all();
	test_semaphore_all();
	test_blocking_queue_all();
	test_blocking_stack_all();
	test_pool_all();
	test_file_all();
	test_arch_net_addr_all();
	test_tcp_all();
	test_stream_tcp_all();
	test_stream_tcp_with_timeout_all();
	test_udp_all();
}

int main(int argc, char* argv[])
{
	test_all();
	printf("ALL TESTS PASSED\n");

	return 0;
}
