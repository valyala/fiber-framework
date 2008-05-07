#include "private/ff_common.h"

#include "private/ff_fiber.h"
#include "private/ff_event.h"
#include "private/ff_core.h"
#include "private/arch/ff_arch_fiber.h"

struct ff_fiber
{
	/* context, which will be passed to the func */
	void *ctx;

	/* the function, which will be executed in the fiber */
	ff_fiber_func func;

	/* the event, which is used by ff_fiber_join() */
	struct ff_event *stop_event;

	/* platform-specific fiber */
	struct ff_arch_fiber *arch_fiber;
};

/**
 * @private
 * The entry point for arch_fiber
 */
static void generic_arch_fiber_func(void *ctx)
{
	struct ff_fiber *fiber;

	fiber = (struct ff_fiber *) ctx;
	fiber->func(fiber->ctx);
	ff_event_set(fiber->stop_event);
	ff_core_yield_fiber();
	ff_assert(0);
}

struct ff_fiber *ff_fiber_initialize()
{
	struct ff_fiber *fiber;

	fiber = (struct ff_fiber *) ff_malloc(sizeof(*fiber));
	fiber->ctx = NULL;
	fiber->func = NULL;
	fiber->stop_event = NULL;
	fiber->arch_fiber = ff_arch_fiber_initialize();

	return fiber;
}

void ff_fiber_shutdown()
{
	struct ff_fiber *current_fiber;
	
	current_fiber = ff_core_get_current_fiber();
	ff_arch_fiber_shutdown(current_fiber->arch_fiber);
	ff_free(current_fiber);
}

void ff_fiber_switch(struct ff_fiber *fiber)
{
	ff_arch_fiber_switch(fiber->arch_fiber);
}

struct ff_fiber *ff_fiber_create(ff_fiber_func fiber_func, int stack_size)
{
	struct ff_fiber *fiber;

	fiber = (struct ff_fiber *) ff_malloc(sizeof(*fiber));
	fiber->ctx = NULL;
	fiber->func = fiber_func;
	fiber->stop_event = ff_event_create(FF_EVENT_MANUAL);
	fiber->arch_fiber = ff_arch_fiber_create(generic_arch_fiber_func, fiber, stack_size);

	return fiber;
}

void ff_fiber_delete(struct ff_fiber *fiber)
{
	ff_arch_fiber_delete(fiber->arch_fiber);
	ff_event_delete(fiber->stop_event);
	ff_free(fiber);
}

void ff_fiber_start(struct ff_fiber *fiber, void *ctx)
{
	fiber->ctx = ctx;
	ff_core_schedule_fiber(fiber);
}

void ff_fiber_join(struct ff_fiber *fiber)
{
	ff_event_wait(fiber->stop_event);
}
