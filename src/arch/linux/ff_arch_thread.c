#include "private/arch/ff_arch_thread.h"
#include "ff_linux_error_check.h"

#include <pthread.h>

struct ff_arch_thread
{
	pthread_t tid;
	pthread_attr_t attr;
	ff_arch_thread_func func;
	void *ctx;
};

static void *generic_thread_func(void *ctx)
{
	struct ff_arch_thread *thread;

	thread = (struct ff_arch_thread *) ctx;
	thread->func(thread->ctx);
	return 0;
}

struct ff_arch_thread *ff_arch_thread_create(ff_arch_thread_func func, int stack_size)
{
	struct ff_arch_thread *thread;
	int rv;

	ff_assert(stack_size >= 0);


	thread = (struct ff_arch_thread *) ff_malloc(sizeof(*thread));
	memset(&thread->tid, 0, sizeof(thread->tid));

	rv = pthread_attr_init(&thread->attr);
	ff_linux_fatal_error_check(rv == 0, L"cannot initialize thread attributes");

	if (stack_size < PTHREAD_STACK_MIN)
	{
		stack_size = PTHREAD_STACK_MIN;
	}
	rv = pthread_attr_setstacksize(&thread->attr, stack_size);
	assert(rv == 0);

	thread->func = func;
	thread->ctx = NULL;

	return thread;
}

void ff_arch_thread_delete(struct ff_arch_thread *thread)
{
	int rv;

	rv = pthread_attr_destroy(&thread->attr);
	ff_assert(rv == 0);

	ff_free(thread);
}

void ff_arch_thread_start(struct ff_arch_thread *thread, void *ctx)
{
	int rv;

	thread->ctx = ctx;
	rv = pthread_create(&thread->tid, &thread->attr, generic_thread_func, thread);
	ff_linux_fatal_error_check(rv ==0, L"cannot start the thread");
}

void ff_arch_thread_join(struct ff_arch_thread *thread)
{
	int rv;
	void *data;

	rv = pthread_join(&thread->tid, &data);
	assert(rv == 0);
}
