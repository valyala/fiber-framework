#include "private/ff_common.h"

#include "private/ff_fiber.h"
#include "private/ff_event.h"
#include "private/ff_core.h"
#include "private/arch/ff_arch_fiber.h"

#define DEFAULT_FIBER_STACK_SIZE 0x10000

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

static struct ff_fiber main_fiber;
static struct ff_fiber *current_fiber = NULL;

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

void ff_fiber_initialize()
{
	ff_assert(current_fiber == NULL);
	main_fiber.ctx = NULL;
	main_fiber.func = NULL;
	main_fiber.stop_event = NULL;
	main_fiber.arch_fiber = ff_arch_fiber_initialize();
	current_fiber = &main_fiber;
}

void ff_fiber_shutdown()
{
	ff_assert(current_fiber == &main_fiber);

	ff_arch_fiber_shutdown(main_fiber.arch_fiber);
	current_fiber = NULL;
}

void ff_fiber_switch(struct ff_fiber *fiber)
{
	if (fiber != current_fiber)
	{
		current_fiber = fiber;
		ff_arch_fiber_switch(fiber->arch_fiber);
	}
}

struct ff_fiber *ff_fiber_create(ff_fiber_func fiber_func, int stack_size)
{
	struct ff_fiber *fiber;

	ff_assert(stack_size >= 0);

	if (stack_size == 0)
	{
		stack_size = DEFAULT_FIBER_STACK_SIZE;
	}

	fiber = (struct ff_fiber *) ff_malloc(sizeof(*fiber));
	fiber->ctx = NULL;
	fiber->func = fiber_func;
	fiber->stop_event = ff_event_create(FF_EVENT_MANUAL);
	fiber->arch_fiber = ff_arch_fiber_create(generic_arch_fiber_func, fiber, stack_size);

	return fiber;
}

void ff_fiber_delete(struct ff_fiber *fiber)
{
	ff_assert(fiber != current_fiber);
	ff_assert(fiber != &main_fiber);

	ff_arch_fiber_delete(fiber->arch_fiber);
	ff_event_delete(fiber->stop_event);
	ff_free(fiber);
}

void ff_fiber_start(struct ff_fiber *fiber, void *ctx)
{
	ff_assert(fiber != current_fiber);
	ff_assert(fiber != &main_fiber);

	fiber->ctx = ctx;
	ff_core_schedule_fiber(fiber);
}

void ff_fiber_join(struct ff_fiber *fiber)
{
	ff_assert(fiber != current_fiber);
	ff_assert(fiber != &main_fiber);

	ff_event_wait(fiber->stop_event);
}

struct ff_fiber *ff_fiber_get_current()
{
	return current_fiber;
}
