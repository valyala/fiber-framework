#ifndef FF_LOG_PUBLIC
#define FF_LOG_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

FF_API void ff_log_info(const wchar_t *format, ...);

FF_API void ff_log_warning(const wchar_t *format, ...);

FF_API void ff_log_fatal_error(const wchar_t *format, ...);

#ifdef __cplusplus
}
#endif

#endif
