#include "private/ff_common.h"

#include "private/arch/ff_arch_lock.h"

#define _WIN32_WINNT 0x0502

#include <windows.h>

static const int SPIN_COUNT = 100;

struct ff_arch_lock
{
	CRITICAL_SECTION critical_section;
};

struct ff_arch_lock *ff_arch_lock_create()
{
	struct ff_arch_lock *lock = (struct ff_arch_lock *) ff_malloc(sizeof(*lock));
	InitializeCriticalSectionAndSpinCount(&lock->critical_section, SPIN_COUNT);
	return lock;
}

void ff_arch_lock_delete(struct ff_arch_lock *lock)
{
	DeleteCriticalSection(&lock->critical_section);
	ff_free(lock);
}

void ff_arch_lock_lock(struct ff_arch_lock *lock)
{
	EnterCriticalSection(&lock->critical_section);
}

void ff_arch_lock_unlock(struct ff_arch_lock *lock)
{
	LeaveCriticalSection(&lock->critical_section);
}
