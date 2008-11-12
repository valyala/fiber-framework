#ifndef FF_LOG_PUBLIC
#define FF_LOG_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ff_log_*() functions should be called only in fiber context after ff_core_initialize()
 * They cannot be called from threadpool thread or outside of the ff_core_initialize() ... ff_core_shutdown()
 */

FF_API void ff_log_debug(const wchar_t *format, ...);

#if defined(NDEBUG)
	/* override ff_log_debug() function */
	#define ff_log_debug(format, ...) /* nothing to do if NDEBUG defined */
#endif

FF_API void ff_log_info(const wchar_t *format, ...);

FF_API void ff_log_warning(const wchar_t *format, ...);

FF_API void ff_log_fatal_error(const wchar_t *format, ...);

#ifdef __cplusplus
}
#endif

#endif
