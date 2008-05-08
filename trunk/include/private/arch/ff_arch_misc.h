#ifndef FF_ARCH_MISC_PRIVATE
#define FF_ARCH_MISC_PRIVATE

#include "private/ff_common.h"
#include "private/arch/ff_arch_completion_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * Initializes architecture subsystem
 */
void ff_arch_misc_initialize(struct ff_arch_completion_port *completion_port);

/**
 * @public
 * Shutdowns architecture subsystem
 */
void ff_arch_misc_shutdown();

/**
 * @public
 * Returns the current time in milliseconds
 */
int64_t ff_arch_misc_get_current_time();

/**
 * @public
 * sleeps for the given interval
 */
void ff_arch_misc_sleep(int interval);

/**
 * @public
 * Returns the number of CPUs in the system
 */
int ff_arch_misc_get_cpus_cnt();

#ifdef __cplusplus
}
#endif

#endif
