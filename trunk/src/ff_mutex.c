#include "private/ff_common.h"

#include "private/ff_mutex.h"
#include "private/ff_stack.h"
#include "private/ff_core.h"

struct ff_mutex
{
	struct ff_stack *pending_fibers;
	int is_locked;
};

struct ff_mutex *ff_mutex_create()
{
	struct ff_mutex *mutex;
	
	mutex = (struct ff_mutex *) ff_malloc(sizeof(*mutex));
	mutex->pending_fibers = ff_stack_create();
	mutex->is_locked = 0;
	return mutex;
}

void ff_mutex_delete(struct ff_mutex *mutex)
{
	ff_assert(!mutex->is_locked);

	ff_stack_delete(mutex->pending_fibers);
	ff_free(mutex);
}

void ff_mutex_lock(struct ff_mutex *mutex)
{
	while (mutex->is_locked)
	{
		struct ff_fiber *current_fiber;

		current_fiber = ff_core_get_current_fiber();
		ff_stack_push(mutex->pending_fibers, current_fiber);
		ff_core_yield_fiber();
	}
	mutex->is_locked = 1;
}

void ff_mutex_unlock(struct ff_mutex *mutex)
{
	int is_empty;

	ff_assert(mutex->is_locked);

	is_empty = ff_stack_is_empty(mutex->pending_fibers);
	if (!is_empty)
	{
		struct ff_fiber *fiber;

		fiber = (struct ff_fiber *) ff_stack_top(mutex->pending_fibers);
		ff_stack_pop(mutex->pending_fibers);
		ff_core_schedule_local_fiber(fiber);
	}
	mutex->is_locked = 0;
}
