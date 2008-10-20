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

/**
 * @public
 * Opens the given file for logging in utf8 mode.
 * Returns NULL if the file cannot be opened.
 */
FILE *ff_arch_misc_open_log_file_utf8(const wchar_t *filename);

/**
 * @public
 * Closes the given stream, which was opened using the ff_arch_misc_open_log_file_utf8()
 */
void ff_arch_misc_close_log_file_utf8(FILE *stream);

#ifdef __cplusplus
}
#endif

#endif
