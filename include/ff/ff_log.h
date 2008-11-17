#ifndef FF_LOG_PUBLIC_H
#define FF_LOG_PUBLIC_H

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
#if !defined(NDEBUG)
	#define ff_log_debug(format, ...) ff_log_debug_private(L"%hs:%d. " format, __FILE__, __LINE__, ## __VA_ARGS__)
#else
	#define ff_log_debug(format, ...) /* nothing to do if NDEBUG is defined */
#endif

/**
 * don't call this function directly! Use ff_log_debug() macro isntead.
 */
FF_API void ff_log_debug_private(const wchar_t *format, ...);

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
