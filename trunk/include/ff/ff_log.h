#ifndef FF_LOG_PUBLIC
#define FF_LOG_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ff_log_level
{
	/* fatal error.
	 * Use this log level in the case of unexpected and unrecoverable event.
	 * The ff_log_write() will shut down the application after logging this error.
	 */
	FF_LOG_ERROR,

	/* warning.
	 * Use this log level in the case of unexpected, but recoverable event.
	 */
	FF_LOG_WARNING,

	/* info.
	 * Use it for logging startup and shutdown info, which can be useful
	 * for application's users or for debugging purposes.
	 */
	FF_LOG_INFO
};

FF_API void ff_log_write(enum ff_log_level level, const wchar_t *format, ...);

#ifdef __cplusplus
}
#endif

#endif
