#include "private/arch/ff_arch_misc.h"
#include "private/arch/ff_arch_completion_port.h"
#include "ff_linux_file.h"
#include "ff_linux_net.h"
#include "ff_linux_error_check.h"
#include "ff_linux_misc.h"

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

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

#define MAX_CPUINFO_LINE_SIZE 0x10000

int ff_arch_misc_get_cpus_cnt()
{
	static int cpus_cnt = 0;

	if (cpus_cnt == 0)
	{
		FILE *fp;
		int rv;
		char *line;

		fp = fopen("/proc/cpuinfo", "r");
		if (fp == NULL)
		{
			cpus_cnt = 1;
			goto end;
		}

		line = (char *) ff_calloc(MAX_CPUINFO_LINE_SIZE, sizeof(line[0]));

		for (;;)
		{
			char *s;

			s = fgets(line, MAX_CPUINFO_LINE_SIZE, fp);
			if (s == NULL)
			{
				break;
			}
			if (memcmp(line, "processor", sizeof("processor") - 1) == 0)
			{
				cpus_cnt++;
			}
		}
		ff_free(line);
		rv = fclose(fp);
		ff_assert(rv == 0);
	}

end:
	ff_assert(cpus_cnt > 0);

	return cpus_cnt;
}

char *ff_linux_misc_wide_to_multibyte_string(const wchar_t *wide_str)
{
	size_t mb_str_len;
	size_t len;
	char *mb_str;

	mb_str_len = wcstombs(NULL, wide_str, 0) + 1;
	ff_assert(mb_str_len != 0);
	mb_str = (char *) ff_calloc(mb_str_len, sizeof(mb_str[0]));
	len = wcstombs(mb_str, wide_str, mb_str_len);
	ff_assert(len + 1 == mb_str_len);

	return mb_str;
}

FILE *ff_arch_misc_open_log_file_utf8(const wchar_t *filename)
{
	FILE *stream;
	char *filename_mb;

	filename_mb = ff_linux_misc_wide_to_multibyte_string(filename);
	stream = fopen(filename_mb, "at");
	ff_free(filename_mb);
	ff_linux_fatal_error_check(stream != NULL, L"cannot open the log file [%ls], errno=%d.", filename, errno);
	return stream;
}

void ff_arch_misc_close_log_file_utf8(FILE *stream)
{
	int rv;

	rv = fclose(stream);
	ff_assert(rv == 0);
}
