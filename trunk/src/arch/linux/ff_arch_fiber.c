#include "private/ff_common.h"

#include "private/arch/ff_arch_fiber.h"

#include <ucontext.h>

static const int DEFAULT_STACK_SIZE = 0x10000;

struct ff_arch_fiber
{
	ucontext_t context;
	void *stack;
};

static struct ff_arch_fiber *current_fiber = NULL;
static struct ff_arch_fiber main_fiber;

struct ff_arch_fiber *ff_arch_fiber_initialize()
{
	struct ff_arch_fiber *fiber;

	fiber = &main_fiber;
	current_fiber = fiber;

	return fiber;
}

void ff_arch_fiber_shutdown(struct ff_arch_fiber *fiber)
{
	ff_assert(fiber == current_fiber);
	ff_assert(fiber == &main_fiber);

	memset(&main_fiber, 0, sizeof(main_fiber));
	current_fiber = NULL;
}

struct ff_arch_fiber *ff_arch_fiber_create(ff_arch_fiber_func arch_fiber_func, void *ctx, int stack_size)
{
	struct ff_arch_fiber *fiber;

	ff_assert(stack_size >= 0);

	if (stack_size == 0)
	{
		stack_size = DEFAULT_STACK_SIZE;
	}

	fiber = (struct ff_arch_fiber *) ff_malloc(sizeof(*fiber));
	fiber->stack = ff_malloc(stack_size);
	getcontext(&fiber->context);
	fiber->context.uc_stack.ss_sp = fiber->stack;
	fiber->context.uc_stack.ss_size = stack_size;
	fiber->context.uc_link = NULL;
	makecontext(&fiber->context, (void (*)()) arch_fiber_func, 1, ctx);

	return fiber;
}

void ff_arch_fiber_delete(struct ff_arch_fiber *fiber)
{
	ff_assert(fiber != current_fiber);
	ff_assert(fiber != &main_fiber);

	ff_free(fiber->stack);
	ff_free(fiber);
}

void ff_arch_fiber_switch(struct ff_arch_fiber *fiber)
{
	ucontext_t *current_context;

	current_context = &current_fiber->context;
	current_fiber = fiber;
	swapcontext(current_context, &fiber->context);
}
