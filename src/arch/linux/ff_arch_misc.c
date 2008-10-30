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
#include <stdlib.h>

struct misc_data
{
	const wchar_t *tmp_dir_path;
	int tmp_dir_path_len;
};

static struct misc_data misc_ctx;

static void initialize_tmp_dir_path()
{
	wchar_t *buf;
	const char *tmp_dir;
	int tmp_dir_len;
	int len;

	tmp_dir = getenv("TMPDIR");
	if (tmp_dir == NULL || tmp_dir[0] == '\0')
	{
		tmp_dir = "/tmp";
	}
	tmp_dir_len = strlen(tmp_dir);
	buf = (wchar_t *) ff_calloc(tmp_dir_len + 2, sizeof(buf[0]));
	len = swprintf(buf, tmp_dir_len + 2, L"%hs/", tmp_dir);
	ff_assert(len == tmp_dir_len + 1);

	misc_ctx.tmp_dir_path = buf;
	misc_ctx.tmp_dir_path_len = len;
}

static void shutdown_tmp_dir_path()
{
	ff_free((void *) misc_ctx.tmp_dir_path);
}

void ff_arch_misc_initialize(struct ff_arch_completion_port *completion_port)
{
	initialize_tmp_dir_path();
	ff_linux_net_initialize(completion_port);
	ff_linux_file_initialize(completion_port);
}

void ff_arch_misc_shutdown()
{
	ff_linux_file_shutdown();
	ff_linux_net_shutdown();
	shutdown_tmp_dir_path();
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

void ff_arch_misc_get_tmp_dir_path(const wchar_t **tmp_dir_path, int *tmp_dir_path_len)
{
	*tmp_dir_path = misc_ctx.tmp_dir_path;
	*tmp_dir_path_len = misc_ctx.tmp_dir_path_len;
}

#define GUID_CSTR_LEN 36

void ff_arch_misc_create_guid_cstr(const wchar_t **guid_cstr, int *guid_cstr_len)
{
	FILE *fp;
	size_t elements_read;
	uint8_t buf[16];
	wchar_t *guid;
	int len;

	fp = fopen("/dev/urandom", "rb");
	ff_linux_fatal_error_check(fp != NULL, L"cannot open the /dev/urandom, errno=%d.", errno);
	elements_read = fread(buf, sizeof(buf[0]), 16, fp);
	ff_linux_fatal_error_check(elements_read == 16, L"cannot read 16 bytes from the /dev/urandom, errno=%d", errno);
	fclose(fp);

	guid = (wchar_t *) ff_calloc(GUID_CSTR_LEN + 1, sizeof(guid[0]));
	len = swprintf(guid, GUID_CSTR_LEN + 1, L"%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
	ff_assert(len == GUID_CSTR_LEN);

	*guid_cstr = guid;
	*guid_cstr_len = GUID_CSTR_LEN;
}

void ff_arch_misc_delete_guid_cstr(const wchar_t *guid_cstr)
{
	ff_free((void *) guid_cstr);
}

void ff_arch_misc_create_unique_file_path(const wchar_t *dir_path, int dir_path_len,
	const wchar_t *prefix, int prefix_len, const wchar_t **unique_file_path, int *unique_file_path_len)
{
	const wchar_t *guid;
	wchar_t *file_path;
	int guid_len;
	int file_path_len;

	ff_assert(dir_path[dir_path_len - 1] == L'/' || dir_path[dir_path_len - 1] == L'\\');

	ff_arch_misc_create_guid_cstr(&guid, &guid_len);
	file_path_len = dir_path_len + prefix_len + guid_len;
	file_path = (wchar_t *) ff_calloc(file_path_len + 1, sizeof(file_path[0]));
	memcpy(file_path, dir_path, dir_path_len * sizeof(dir_path[0]));
	memcpy(file_path + dir_path_len, prefix, prefix_len * sizeof(prefix[0]));
	memcpy(file_path + dir_path_len + prefix_len, guid, guid_len * sizeof(guid[0]));
	ff_arch_misc_delete_guid_cstr(guid);

	*unique_file_path = file_path;
	*unique_file_path_len = file_path_len;
}

void ff_arch_misc_delete_unique_file_path(const wchar_t *unique_file_path)
{
	ff_free((void *) unique_file_path);
}
