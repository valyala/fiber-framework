#include "private/arch/ff_arch_misc.h"
#include "private/arch/ff_arch_completion_port.h"
#include "ff_linux_file.h"
#include "ff_linux_net.h"
#include "ff_linux_error_check.h"
#include "ff_linux_misc.h"

#include <time.h>
#include <sys/time.h>

void ff_arch_misc_initialize(struct ff_arch_completion_port *completion_port)
{
	ff_linux_net_initialize(completion_port);
	ff_linux_file_initialize(completion_port);
}

void ff_arch_misc_shutdown()
{
	ff_linux_file_shutdown();
	ff_linux_net_shutdown();
}

int64_t ff_arch_misc_get_current_time()
{
	struct timeval tv;
	int rv;
	int64_t current_time;

	rv = gettimeofday(&tv, NULL);
	current_time = (((int64_t) tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
	return current_time;
}

void ff_arch_misc_sleep(int interval)
{
	int rv;
	struct timespec sleep_time;
	struct timespec remained_time;

	sleep_time.tv_sec = interval / 1000;
	sleep_time.tv_nsec = (interval % 1000) * 1000000;

	for (;;)
	{
		rv = nanosleep(&sleep_time, &remained_time);
		if (rv != -1)
		{
			break;
		}
		ff_assert(errno == EINTR);
		memcpy(&sleep_time, &remained_time, sizeof(sleep_time));
	}
}

int ff_arch_misc_get_cpus_cnt()
{
	return 1;
}

char *ff_linux_misc_wide_to_multibyte_string(const wchar_t *wide_str)
{
	size_t mb_str_len;
	size_t len;
	char *mb_str;

	mb_str_len = wcstombs(NULL, wide_str, 0) + 1;
	ff_assert(mb_str_len != 0);
	mb_str = (char *) ff_malloc(mb_str_len);
	len = wcstombs(mb_str, wide_str, mb_str_len);
	ff_assert(len + 1 == mb_str_len);

	return mb_str;
}