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
 * Represents opaque data, associated with current timeout operation
 */
struct ff_core_timeout_operation_data;

/**
 * @public
 * Registers the timeout operation.
 * Returns the data, which should be passed to ff_core_deregister_timeout_operation()
 * after operation completion.
 */
struct ff_core_timeout_operation_data *ff_core_register_timeout_operation(int timeout, ff_core_cancel_timeout_func cancel_timeout_func, void *ctx);

/**
 * @public
 * Deregisters the timeout operation, which was registered using ff_core_register_timeout_operation()
 * Returns 1 on success, 0 on error.
 */
int ff_core_deregister_timeout_operation(struct ff_core_timeout_operation_data *timeout_operation_data);

#ifdef __cplusplus
}
#endif

#endif
