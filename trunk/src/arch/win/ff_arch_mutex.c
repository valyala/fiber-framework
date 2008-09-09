#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_mutex.h"

static const int SPIN_COUNT = 100;

struct ff_arch_mutex
{
	CRITICAL_SECTION critical_section;
};

struct ff_arch_mutex *ff_arch_mutex_create()
{
	struct ff_arch_mutex *mutex;
	BOOL rv;

	mutex = (struct ff_arch_mutex *) ff_malloc(sizeof(*mutex));
	rv = InitializeCriticalSectionAndSpinCount(&mutex->critical_section, SPIN_COUNT);
	ff_assert(rv != FALSE);

	return mutex;
}

void ff_arch_mutex_delete(struct ff_arch_mutex *mutex)
{
	DeleteCriticalSection(&mutex->critical_section);
	ff_free(mutex);
}

void ff_arch_mutex_lock(struct ff_arch_mutex *mutex)
{
	EnterCriticalSection(&mutex->critical_section);
}

void ff_arch_mutex_unlock(struct ff_arch_mutex *mutex)
{
	LeaveCriticalSection(&mutex->critical_section);
}
