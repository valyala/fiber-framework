#include "private/ff_common.h"

#include "private/ff_lock.h"
#include "private/ff_stack.h"
#include "private/ff_core.h"

struct ff_lock
{
	int is_locked;
	struct ff_stack *pending_fibers;
};

struct ff_lock *ff_lock_create()
{
	struct ff_lock *lock = (struct ff_lock *) ff_malloc(sizeof(*lock));
	lock->is_locked = 0;
	lock->pending_fibers = ff_stack_create();
	return lock;
}

void ff_lock_delete(struct ff_lock *lock)
{
	ff_assert(!lock->is_locked);

	ff_stack_delete(lock->pending_fibers);
	ff_free(lock);
}

void ff_lock_lock(struct ff_lock *lock)
{
	while (lock->is_locked)
	{
		struct ff_fiber *current_fiber = ff_core_get_current_fiber();
		ff_stack_push(lock->pending_fibers, current_fiber);
		ff_core_yield_fiber();
	}
	lock->is_locked = 1;
}

void ff_lock_unlock(struct ff_lock *lock)
{
	int is_empty;

	ff_assert(lock->is_locked);

	is_empty = ff_stack_is_empty(lock->pending_fibers);
	if (!is_empty)
	{
		struct ff_fiber *fiber = (struct ff_fiber *) ff_stack_top(lock->pending_fibers);
		ff_stack_pop(lock->pending_fibers);
		ff_core_schedule_local_fiber(fiber);
	}
	lock->is_locked = 0;
}
