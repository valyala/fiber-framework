#ifndef FF_FIBER_PRIVATE
#define FF_FIBER_PRIVATE

#include "ff/ff_fiber.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * Initializes the fiber framework.
 */
void ff_fiber_initialize();

/**
 * @public
 * Shutdowns the fiber framework
 */
void ff_fiber_shutdown();

/**
 * @public
 * Switches to the given fiber
 */
void ff_fiber_switch(struct ff_fiber *fiber);

#ifdef __cplusplus
}
#endif

#endif
