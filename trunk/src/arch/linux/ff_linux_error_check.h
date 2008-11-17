#ifndef FF_LINUX_ERROR_CHECK_H
#define FF_LINUX_ERROR_CHECK_H

#include "private/ff_common.h"

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ff_linux_fatal_error_check(expression, format, ...) \
	do { \
		if (!(expression)) \
		{ \
			ff_log_fatal_error(L"fatal error at %hs:%d, errno=%d. " format, __FILE__, __LINE__, errno, ## __VA_ARGS__); \
		} \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif
