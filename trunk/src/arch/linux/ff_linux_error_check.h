#ifndef FF_LINUX_ERROR_CHECK
#define FF_LINUX_ERROR_CHECK

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

void fatal_error(char *format, ...);

#define ff_linux_fatal_error_check(expression, format, ...) \
	do { \
		if (!(expression)) \
		{ \
			fatal_error("WINAPI fatal error at %s:%d, errno=%d. " format, __FILE__, __LINE__, errno, ## __VA_ARGS__); \
		} \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif
