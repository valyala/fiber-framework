#include "private/ff_common.h"

#include "private/ff_log.h"
#include "private/arch/ff_arch_misc.h"

#include <stdarg.h>

struct ff_log_ctx
{
	FILE *log_file;
};

static struct ff_log_ctx log_data;
static int is_log_initialized = 0;

static void write_log(const char *log_level, const wchar_t *format, va_list args_ptr)
{
	int len;
	int rv;

	len = fwprintf(log_data.log_file, L"%hs: ", log_level);
	ff_assert(len > 0);
   	len = vfwprintf(log_data.log_file, format, args_ptr);
   	ff_assert(len >= 0);
   	len = fwprintf(log_data.log_file, L"\n");
   	ff_assert(len > 0);
   	rv = fflush(log_data.log_file);
   	ff_assert(rv == 0);
}

void ff_log_initialize(const wchar_t *log_filename)
{
	ff_assert(!is_log_initialized);
	log_data.log_file = ff_arch_misc_open_log_file_utf8(log_filename);
	if (log_data.log_file == NULL)
	{
		fwprintf(stderr, L"cannot open the file [%ls]", log_filename);
		exit(EXIT_FAILURE);
	}
	is_log_initialized = 1;
}

void ff_log_shutdown()
{
	ff_assert(is_log_initialized);
	ff_arch_misc_close_log_file_utf8(log_data.log_file);
	is_log_initialized = 0;
}

void ff_log_info(const wchar_t *format, ...)
{
	va_list args_ptr;

	ff_assert(is_log_initialized);
	va_start(args_ptr, format);
	write_log("info", format, args_ptr);
	va_end(args_ptr);
}

void ff_log_warning(const wchar_t *format, ...)
{
	va_list args_ptr;

	ff_assert(is_log_initialized);
	va_start(args_ptr, format);
	write_log("warning", format, args_ptr);
	va_end(args_ptr);
}

void ff_log_fatal_error(const wchar_t *format, ...)
{
	va_list args_ptr;

	ff_assert(is_log_initialized);
	va_start(args_ptr, format);
	write_log("error", format, args_ptr);
	va_end(args_ptr);

	exit(EXIT_FAILURE);
}
