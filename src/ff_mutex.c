#include "private/ff_common.h"

#include "private/ff_mutex.h"
#include "private/ff_stack.h"
#include "private/ff_core.h"
#include "private/ff_fiber.h"

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
	struct ff_stack *pending_fibers;

	pending_fibers = mutex->pending_fibers;
	while (mutex->is_locked)
	{
		struct ff_fiber *current_fiber;

		current_fiber = ff_fiber_get_current();
		ff_stack_push(pending_fibers, current_fiber);
		ff_core_yield_fiber();
	}
	mutex->is_locked = 1;
}

void ff_mutex_unlock(struct ff_mutex *mutex)
{
	struct ff_stack *pending_fibers;
	int is_empty;

	ff_assert(mutex->is_locked);
	pending_fibers = mutex->pending_fibers;
	is_empty = ff_stack_is_empty(pending_fibers);
	if (!is_empty)
	{
		struct ff_fiber *fiber;

		ff_stack_top(pending_fibers, (const void **) &fiber);
		ff_stack_pop(pending_fibers);
		ff_core_schedule_fiber(fiber);
	}
	mutex->is_locked = 0;
}
