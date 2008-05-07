#ifndef FF_CORE_PRIVATE
#define FF_CORE_PRIVATE

#include "public/ff_core.h"
#include "private/ff_fiber.h"
#include "private/arch/ff_arch_completion_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * Schedules the given fiber for execution
 */
void ff_core_schedule_fiber(struct ff_fiber *fiber);

/**
 * @public
 * Yields the current fiber
 */
void ff_core_yield_fiber();

/**
 * @public
 * the function, which is called when cancelling the timed out operation
 */
typedef void (*ff_core_cancel_timeout_func)(struct ff_fiber *fiber, void *ctx);

/**
 * @public
 * performs timeout operation.
 * Returns 0 if the operation was timed out, otherwise returns 0
 */
int ff_core_do_timeout_operation(int timeout, ff_core_cancel_timeout_func cancel_timeout_func, void *ctx);

/**
 * @public
 * Returns the current fiber
 */
struct ff_fiber *ff_core_get_current_fiber();

#ifdef __cplusplus
}
#endif

#endif
