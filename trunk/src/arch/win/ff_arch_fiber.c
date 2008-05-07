#include "private/ff_common.h"

#include "private/arch/ff_arch_fiber.h"
#include "ff_win_error_check.h"

#define _WIN32_WINNT 0x0502

#include <windows.h>

struct ff_arch_fiber
{
	LPVOID handle;
};

struct ff_arch_fiber *ff_arch_fiber_initialize()
{
	struct ff_arch_fiber *fiber;

	fiber = (struct ff_arch_fiber *) ff_malloc(sizeof(*fiber));
	fiber->handle = ConvertThreadToFiber((LPVOID) NULL);
	ff_assert(fiber->handle != NULL);
	return fiber;
}

void ff_arch_fiber_shutdown(struct ff_arch_fiber *fiber)
{
	BOOL rv;

	rv = ConvertFiberToThread();
	ff_assert(rv != FALSE);
	ff_free(fiber);
}

struct ff_arch_fiber *ff_arch_fiber_create(ff_arch_fiber_func arch_fiber_func, void *ctx, int stack_size)
{
	struct ff_arch_fiber *fiber;

	fiber = (struct ff_arch_fiber *) ff_malloc(sizeof(*fiber));
	fiber->handle = CreateFiber(stack_size, (LPFIBER_START_ROUTINE) arch_fiber_func, ctx);
	ff_winapi_fatal_error_check(fiber->handle != NULL, "cannot create new fiber");
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
