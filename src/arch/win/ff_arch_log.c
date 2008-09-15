#include "private/ff_common.h"

#include "private/arch/ff_arch_log.h"
#include "ff_linux_error_check.h"
#include "ff_linux_misc.h"

#define LOG_LEVEL_TO_STR(level) ((level) == FF_LOG_ERROR ? L"error" : ((level) == FF_LOG_WARNING ? L"warning" : L"info"))

struct ff_arch_log_ctx
{
	FILE *log_file;
};

static struct ff_arch_log_ctx arch_log_data;
static int is_arch_log_initialized = 0;

static void write_log_to_stream(FILE *stream, enum ff_log_level level, const wchar_t *format, va_list args_ptr)
{
	int len;
	int rv;
	const wchar_t *level_str;

	level_str = LOG_LEVEL_TO_STR(level);
	ff_assert(level_str != NULL);
	len = fwprintf(stream, L"%ls: ", level_str);
	ff_assert(len > 0);
   	len = vfwprintf(stream, format, args_ptr);
   	ff_assert(len >= 0);
   	len = fwprintf_s(stream, L"\n");
   	ff_assert(len > 0);
   	rv = fflush(stream);
   	ff_assert(rv == 0);
}

void ff_arch_log_initialize(const wchar_t *log_filename)
{
	const char *log_filename_mb;

	ff_assert(!is_arch_log_initialized);
	log_filename_mb = ff_linux_misc_wide_to_multibyte_string(log_filename);
	arch_log_data.log_file = fopen(log_filename_mb, "at");
	ff_free(log_filename_mb);
	ff_linux_fatal_error_check(arch_log_data.log_file != NULL, L"cannot open the log file [%ls], errno=%d.", log_filename, errno);
	is_arch_log_initialized = 1;
}

void ff_arch_log_shutdown()
{
	int rv;

	ff_assert(is_arch_log_initialized);
	rv = fclose(arch_log_data.log_file);
	ff_assert(rv == 0);
	is_arch_log_initialized = 0;
}

void ff_arch_log_write(enum ff_log_level level, const wchar_t *format, va_list args_ptr)
{
	FILE *log_stream;

	log_stream = is_arch_log_initialized ? arch_log_data.log_file : stderr;
	write_log_to_stream(log_stream, level, format, args_ptr);
}
