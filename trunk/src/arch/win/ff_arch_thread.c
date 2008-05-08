#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_thread.h"
#include "ff_win_error_check.h"

#include <process.h>

struct ff_arch_thread
{
	HANDLE handle;
	ff_arch_thread_func func;
	void *ctx;
};

static unsigned int WINAPI generic_thread_func(void *ctx)
{
	struct ff_arch_thread *thread;

	thread = (struct ff_arch_thread *) ctx;
	thread->func(thread->ctx);
	return 0;
}

struct ff_arch_thread *ff_arch_thread_create(ff_arch_thread_func func, int stack_size)
{
	struct ff_arch_thread *thread;

	thread = (struct ff_arch_thread *) ff_malloc(sizeof(*thread));
	thread->handle = (HANDLE) _beginthreadex(NULL, stack_size, generic_thread_func, thread, CREATE_SUSPENDED, NULL);
    ff_winapi_fatal_error_check(thread->handle != NULL, L"cannot start new thread");
	thread->func = func;
	thread->ctx = NULL;
	return thread;
}

void ff_arch_thread_delete(struct ff_arch_thread *thread)
{
	BOOL result;
	
	result = CloseHandle(thread->handle);
	ff_assert(result != FALSE);
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
	DWORD result;

	result = WaitForSingleObject(thread->handle, INFINITE);
	ff_assert(result == WAIT_OBJECT_0);
}
