#include "private/ff_common.h"

#include "private/arch/ff_arch_thread.h"

//#define _WIN32_WINNT 0x0502

#include <windows.h>
#include <process.h>

struct ff_arch_thread
{
	HANDLE handle;
	ff_arch_thread_func func;
	void *ctx;
};

static unsigned int WINAPI generic_thread_func(void *ctx)
{
	struct ff_arch_thread *thread = (struct ff_arch_thread *) ctx;
	thread->func(thread->ctx);
	return 0;
}

struct ff_arch_thread *ff_arch_thread_create(ff_arch_thread_func func, int stack_size)
{
	struct ff_arch_thread *thread = (struct ff_arch_thread *) ff_malloc(sizeof(*thread));
	thread->handle = (HANDLE) _beginthreadex(NULL, stack_size, generic_thread_func, thread, CREATE_SUSPENDED, NULL);
	thread->func = func;
	thread->ctx = NULL;
	return thread;
}

void ff_arch_thread_delete(struct ff_arch_thread *thread)
{
	BOOL rv = CloseHandle(thread->handle);
	ff_assert(rv != FALSE);
	ff_free(thread);
}

void ff_arch_thread_start(struct ff_arch_thread *thread, void *ctx)
{
	DWORD result;

	thread->ctx = ctx;
	result = ResumeThread(thread->handle);
	ff_assert(result != (DWORD)-1);
}

void ff_arch_thread_join(struct ff_arch_thread *thread)
{
	DWORD result = WaitForSingleObject(thread->handle, INFINITE);
	ff_assert(result == WAIT_OBJECT_0);
}
