#ifndef FF_ARCH_LOG_PRIVATE
#define FF_ARCH_LOG_PRIVATE

#include "private/ff_log.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void ff_arch_log_initialize(const wchar_t *log_filename);

void ff_arch_log_shutdown();

void ff_arch_log_write(enum ff_log_level level, const wchar_t *format, va_list args_ptr);

#ifdef __cplusplus
}
#endif

#endif
