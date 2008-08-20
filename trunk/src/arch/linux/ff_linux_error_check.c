#include "private/ff_common.h"

#include "ff_linux_error_check.h"
#include "ff_linux_misc.h"

#include <stdarg.h>

void fatal_error(const wchar_t *format, ...)
{
	int len;
	char *mb_format;

	mb_format = ff_linux_misc_wide_to_multibyte_string(format);

	va_list args_ptr;
	va_start(args_ptr, format);
	len = vfprintf(stderr, mb_format, args_ptr);
	ff_assert(len >= 0);
	va_end(args_ptr);

	ff_free(mb_format);
	exit(EXIT_FAILURE);
}
