#include "private/ff_common.h"

#include "private/ff_log.h"
#include "private/arch/ff_arch_misc.h"
#include "private/arch/ff_arch_mutex.h"

#include <stdarg.h>

struct ff_log_ctx
{
	struct ff_arch_mutex *mutex;
	FILE *log_file;
};

static struct ff_log_ctx log_data;
static int is_log_initialized = 0;

static void write_log_to_stream(FILE *stream, const char *log_level, const wchar_t *format, va_list args_ptr)
{
	int len;
	int rv;

	if (is_log_initialized)
	{
		ff_arch_mutex_lock(log_data.mutex);
	}
	len = fwprintf(stream, L"%hs: ", log_level);
	ff_assert(len > 0);
   	len = vfwprintf(stream, format, args_ptr);
   	ff_assert(len >= 0);
   	len = fwprintf(stream, L"\n");
   	ff_assert(len > 0);
   	rv = fflush(stream);
   	ff_assert(rv == 0);
	if (is_log_initialized)
	{
		ff_arch_mutex_unlock(log_data.mutex);
	}
}

void ff_log_initialize(const wchar_t *log_filename)
{
	ff_assert(!is_log_initialized);
	log_data.mutex = ff_arch_mutex_create();
	log_data.log_file = ff_arch_misc_open_log_file_utf8(log_filename);
	if (log_data.log_file == NULL)
	{
		ff_log_fatal_error(L"cannot open the file [%ls]", log_filename);
	}
	is_log_initialized = 1;
}

void ff_log_shutdown()
{
	ff_assert(is_log_initialized);
	ff_arch_misc_close_log_file_utf8(log_data.log_file);
	ff_arch_mutex_delete(log_data.mutex);
	is_log_initialized = 0;
}

void ff_log_info(const wchar_t *format, ...)
{
	FILE *log_stream;
	va_list args_ptr;

	log_stream = is_log_initialized ? log_data.log_file : stderr;
	va_start(args_ptr, format);
	write_log_to_stream(log_stream, "info", format, args_ptr);
	va_end(args_ptr);
}

void ff_log_warning(const wchar_t *format, ...)
{
	FILE *log_stream;
	va_list args_ptr;

	log_stream = is_log_initialized ? log_data.log_file : stderr;
	va_start(args_ptr, format);
	write_log_to_stream(log_stream, "warning", format, args_ptr);
	va_end(args_ptr);
}

void ff_log_fatal_error(const wchar_t *format, ...)
{
	FILE *log_stream;
	va_list args_ptr;

	log_stream = is_log_initialized ? log_data.log_file : stderr;
	va_start(args_ptr, format);
	write_log_to_stream(log_stream, "error", format, args_ptr);
	va_end(args_ptr);

	exit(EXIT_FAILURE);
}
