#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_misc.h"
#include "private/arch/ff_arch_completion_port.h"
#include "ff_win_net.h"
#include "ff_win_file.h"
#include "ff_win_error_check.h"

#include <share.h>
#include <Rpc.h> /* for generating GUIDs */

struct misc_data
{
	const wchar_t *tmp_dir_path;
	int tmp_dir_path_len;
};

static struct misc_data misc_ctx;

static void initialize_tmp_dir_path()
{
	DWORD buf_len;
	wchar_t *buf = NULL;

	buf_len = GetTempPathW(0, buf);
	ff_winapi_fatal_error_check(buf_len > 0, L"GetTempPathW() failed when calculating required buffer size");
	buf = (wchar_t *) ff_calloc(buf_len + 1, sizeof(buf[0]));
	misc_ctx.tmp_dir_path_len = (int) GetTempPathW(buf_len, buf);
	ff_winapi_fatal_error_check(misc_ctx.tmp_dir_path_len + 1 == buf_len, L"GetTempPathW() failed when copying tmp path to allocated buffer");
	misc_ctx.tmp_dir_path = buf;
}

static void shutdown_tmp_dir_path()
{
	ff_free((void *) misc_ctx.tmp_dir_path);
}

void ff_arch_misc_initialize(struct ff_arch_completion_port *completion_port)
{
	initialize_tmp_dir_path();
	ff_win_net_initialize(completion_port);
	ff_win_file_initialize(completion_port);
}

void ff_arch_misc_shutdown()
{
	ff_win_file_shutdown();
	ff_win_net_shutdown();
	shutdown_tmp_dir_path();
}

int64_t ff_arch_misc_get_current_time()
{
	int64_t current_time;

	current_time = GetTickCount();
	return current_time;
}

void ff_arch_misc_sleep(int interval)
{
	Sleep(interval);
}

int ff_arch_misc_get_cpus_cnt()
{
	int cpus_cnt;
	SYSTEM_INFO system_info;

	GetSystemInfo(&system_info);
	cpus_cnt = (int) system_info.dwNumberOfProcessors;
	return cpus_cnt;
}

FILE *ff_arch_misc_open_log_file_utf8(const wchar_t *filename)
{
	FILE *stream;
	
	stream = _wfsopen(filename, L"at, ccs=UTF-8", _SH_DENYWR);
	ff_winapi_fatal_error_check(stream != NULL, L"cannot open the log file [%ls], errno=%d.", filename, errno);
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

void ff_arch_misc_create_guid_cstr(const wchar_t **guid_cstr, int *guid_cstr_len)
{
	UUID guid;
	RPC_STATUS status;
	RPC_WSTR guid_str;
	size_t guid_str_len;

	status = UuidCreate(&guid);
	ff_winapi_fatal_error_check(status == RPC_S_OK || status == RPC_S_UUID_LOCAL_ONLY, L"UuidCreate() failed");
	status = UuidToStringW(&guid, &guid_str);
	ff_winapi_fatal_error_check(status == RPC_S_OK, L"UuidToStringW() failed");
	guid_str_len = wcslen((wchar_t *) guid_str);

	*guid_cstr = (const wchar_t *) guid_str;
	*guid_cstr_len = (int) guid_str_len;
	ff_assert(*guid_cstr_len > 0);
}

void ff_arch_misc_delete_guid_cstr(const wchar_t *guid_cstr)
{
	RPC_STATUS status;

	status = RpcStringFreeW((RPC_WSTR *) &guid_cstr);
	ff_assert(status == RPC_S_OK);
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
