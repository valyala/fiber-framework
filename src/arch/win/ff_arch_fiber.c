#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_fiber.h"

struct ff_arch_fiber
{
	LPVOID handle;
};

static struct ff_arch_fiber main_fiber;

struct ff_arch_fiber *ff_arch_fiber_initialize()
{
	main_fiber.handle = ConvertThreadToFiber((LPVOID) NULL);
	ff_assert(main_fiber.handle != NULL);

	return &main_fiber;
}

void ff_arch_fiber_shutdown(struct ff_arch_fiber *fiber)
{
	BOOL rv;

	ff_assert(fiber == &main_fiber);

	rv = ConvertFiberToThread();
	ff_assert(rv != FALSE);
}

struct ff_arch_fiber *ff_arch_fiber_create(ff_arch_fiber_func arch_fiber_func, void *ctx, int stack_size)
{
	struct ff_arch_fiber *fiber;

	fiber = (struct ff_arch_fiber *) ff_malloc(sizeof(*fiber));
	fiber->handle = CreateFiber(stack_size, (LPFIBER_START_ROUTINE) arch_fiber_func, ctx);
	ff_winapi_fatal_error_check(fiber->handle != NULL, L"cannot create new fiber");
	return fiber;
}

void ff_arch_fiber_delete(struct ff_arch_fiber *fiber)
{
	DeleteFiber(fiber->handle);
	ff_free(fiber);
}

void ff_arch_fiber_switch(struct ff_arch_fiber *fiber)
{
	SwitchToFiber(fiber->handle);
}
