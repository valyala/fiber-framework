#include "private/ff_common.h"

#include "private/ff_log.h"
#include "private/arch/ff_arch_log.h"
#include "private/arch/ff_arch_mutex.h"

#include <stdarg.h>

struct ff_log_ctx
{
	struct ff_arch_mutex *mutex;
};

static struct ff_log_ctx log_data;
static int is_log_initialized = 0;
static PER_THREAD is_log_write_called = 0;

void ff_log_initialize(const wchar_t *log_filename)
{
	ff_assert(!is_log_initialized);
	log_data.mutex = ff_arch_mutex_create();
	ff_arch_log_initialize(log_filename);
	is_log_initialized = 1;
}

void ff_log_shutdown()
{
	ff_assert(is_log_initialized);
	ff_arch_log_shutdown();
	ff_arch_mutex_delete(log_data.mutex);
	is_log_initialized = 0;
}

void ff_log_write(enum ff_log_level level, const wchar_t *format, ...)
{
	va_list args_ptr;

	if (is_log_write_called)
	{
		fprintf(stderr, "recursive call to the ff_log_write() detected\n");
		exit(EXIT_FAILURE);
	}

	is_log_write_called = 1;
	if (is_log_initialized)
	{
		ff_arch_mutex_lock(log_data.mutex);
	}

	/* even if is_log_initialized == 0, then the ff_arch_log_write() should
	 * write the log message somewhere (at least to the stderr).
	 */
	va_start(args_ptr, format);
	ff_arch_log_write(level, format, args_ptr);
	if (level == FF_LOG_ERROR)
	{
		/* shut down the application on fatal error */
		exit(EXIT_FAILURE);
	}
	if (is_log_initialized)
	{
		ff_arch_mutex_unlock(log_data.mutex);
	}
	is_log_write_called = 0;
}
