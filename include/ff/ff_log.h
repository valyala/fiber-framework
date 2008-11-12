#ifndef FF_LOG_PUBLIC
#define FF_LOG_PUBLIC

#include <wchar.h>

#include "ff/ff_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ff_log_*() functions should be called only after ff_core_initialize() was called.
 * These functions must be called either in fiber context either from the threadpool thread.
 */

/**
 * logs debug message. If the NDEBUG is defined, then this call transforms to an empty statement.
 * See ff_log_debug macro definition below.
 */
FF_API void ff_log_debug(const wchar_t *format, ...);

#if defined(NDEBUG)
	/* override ff_log_debug() function */
	#define ff_log_debug(format, ...) /* nothing to do if NDEBUG defined */
#endif

/**
 * logs info message.
 */
FF_API void ff_log_info(const wchar_t *format, ...);

/**
 * logs warning message.
 */
FF_API void ff_log_warning(const wchar_t *format, ...);

/**
 * logs fatal error message and shutdowns the current application.
 */
FF_API void ff_log_fatal_error(const wchar_t *format, ...);

#ifdef __cplusplus
}
#endif

#endif
