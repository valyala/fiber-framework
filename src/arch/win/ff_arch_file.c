#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_file.h"
#include "private/ff_core.h"
#include "private/ff_fiber.h"
#include "ff_win_completion_port.h"

struct ff_arch_file
{
	HANDLE handle;
	int64_t curr_pos;
};

struct threadpool_open_file_data
{
	const wchar_t *path;
	enum ff_arch_file_access_mode access_mode;
	HANDLE handle;
};

struct threadpool_erase_file_data
{
	const wchar_t *path;
	enum ff_result result;
};

struct threadpool_copy_file_data
{
	const wchar_t *src_path;
	const wchar_t *dst_path;
	enum ff_result result;
};

struct threadpool_move_file_data
{
	const wchar_t *src_path;
	const wchar_t *dst_path;
	enum ff_result result;
};

struct file_data
{
	struct ff_arch_completion_port *completion_port;
};

static struct file_data file_ctx;

static void threadpool_open_file_func(void *ctx)
{
	struct threadpool_open_file_data *data;
	DWORD access_rights;
	DWORD share_mode;
	DWORD creation_disposition;

	data = (struct threadpool_open_file_data *) ctx;
	if (data->access_mode == FF_ARCH_FILE_READ)
	{
		access_rights = GENERIC_READ;
		share_mode = FILE_SHARE_READ;
		creation_disposition = OPEN_EXISTING;
	}
	else
	{
		ff_assert(data->access_mode == FF_ARCH_FILE_WRITE);
		access_rights = GENERIC_WRITE;
		share_mode = 0;
		creation_disposition = CREATE_ALWAYS;
	}
	data->handle = CreateFileW(data->path, access_rights, share_mode, NULL, creation_disposition,
		FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (data->handle == INVALID_HANDLE_VALUE)
	{
		DWORD last_error;
		const char *mode;

		mode = (data->access_mode == FF_ARCH_FILE_WRITE) ? "writing" : "reading";
		last_error = GetLastError();
		ff_log_debug(L"cannot open the file [%ls] for %hs. GetLastError()=%lu", data->path, mode, last_error);
	}
}

static void threadpool_erase_file_func(void *ctx)
{
	struct threadpool_erase_file_data *data;
	BOOL result;

	data = (struct threadpool_erase_file_data *) ctx;
	result = DeleteFileW(data->path);
	if (result == FALSE)
	{
		DWORD last_error;

		last_error = GetLastError();
		ff_log_debug(L"cannot delete the file [%ls]. GetLastError()=%lu", data->path, last_error);
	}
	data->result = (result == FALSE) ? FF_FAILURE : FF_SUCCESS;
}

static void threadpool_copy_file_func(void *ctx)
{
	struct threadpool_copy_file_data *data;
	BOOL result;

	data = (struct threadpool_copy_file_data *) ctx;
	result = CopyFileW(data->src_path, data->dst_path, TRUE);
	if (result == FALSE)
	{
		DWORD last_error;

		last_error = GetLastError();
		ff_log_debug(L"cannot copy the file [%ls] to the [%ls]. GetLastError()=%lu", data->src_path, data->dst_path, last_error);
	}
	data->result = (result == FALSE) ? FF_FAILURE : FF_SUCCESS;
}

static void threadpool_move_file_func(void *ctx)
{
	struct threadpool_move_file_data *data;
	BOOL result;

	data = (struct threadpool_move_file_data *) ctx;
	result = MoveFileEx(data->src_path, data->dst_path, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	if (result == FALSE)
	{
		DWORD last_error;

		last_error = GetLastError();
		ff_log_debug(L"cannot move the file [%ls] to the [%ls]. GetLastError()=%lu", data->src_path, data->dst_path, last_error);
	}
	data->result = (result == FALSE) ? FF_FAILURE : FF_SUCCESS;
}

static int complete_overlapped_io(struct ff_arch_file *file, OVERLAPPED *overlapped)
{
	struct ff_fiber *current_fiber;
	int int_bytes_transferred = -1;
	BOOL result;
	DWORD bytes_transferred;

	current_fiber = ff_fiber_get_current();
	ff_win_completion_port_register_overlapped_data(file_ctx.completion_port, overlapped, current_fiber);
	ff_core_yield_fiber();
	ff_win_completion_port_deregister_overlapped_data(file_ctx.completion_port, overlapped);

	result = GetOverlappedResult(file->handle, overlapped, &bytes_transferred, FALSE);
	if (result != FALSE)
	{
		int_bytes_transferred = (int) bytes_transferred;
		file->curr_pos += int_bytes_transferred;
	}
	else
	{
		DWORD last_error;

		last_error = GetLastError();
		ff_log_debug(L"the overlapped operation on the file=%llu, overlapped=%p failed. GetLastError()=%lu", (uint64_t) file->handle, overlapped, last_error);
	}

	return int_bytes_transferred;
}

void ff_win_file_initialize(struct ff_arch_completion_port *completion_port)
{
	file_ctx.completion_port = completion_port;
}

void ff_win_file_shutdown()
{
	/* nothing to do */
}

struct ff_arch_file *ff_arch_file_open(const wchar_t *path, enum ff_arch_file_access_mode access_mode)
{
	struct ff_arch_file *file = NULL;
	struct threadpool_open_file_data data;

	data.path = path;
	data.access_mode = access_mode;
	data.handle = INVALID_HANDLE_VALUE;
	ff_core_threadpool_execute(threadpool_open_file_func, &data);
	if (data.handle == INVALID_HANDLE_VALUE)
	{
		const char *mode;

		mode = (access_mode == FF_ARCH_FILE_WRITE) ? "writing" : "reading";
		ff_log_debug(L"cannot open the file [%ls] for %hs. See previous messages for more info", path, mode);
		goto end;
	}

	file = (struct ff_arch_file *) ff_malloc(sizeof(*file));
	file->handle = data.handle;
	file->curr_pos = 0;

	ff_win_completion_port_register_handle(file_ctx.completion_port, file->handle);

end:
	return file;
}

void ff_arch_file_close(struct ff_arch_file *file)
{
	BOOL result;

	result = CloseHandle(file->handle);
	ff_assert(result != FALSE);
	ff_free(file);
}

int ff_arch_file_read(struct ff_arch_file *file, void *buf, int len)
{
	OVERLAPPED overlapped;
	BOOL result;
	DWORD bytes_read;
	int int_bytes_read = -1;

	ff_assert(len >= 0);

	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.Offset = (DWORD) file->curr_pos;
	overlapped.OffsetHigh = (DWORD) (file->curr_pos >> 32);
	result = ReadFile(file->handle, buf, (DWORD) len, &bytes_read, &overlapped);
	if (result == FALSE)
	{
		DWORD last_error;
		
		last_error = GetLastError();
		if (last_error != ERROR_IO_PENDING)
		{
			if (last_error == ERROR_HANDLE_EOF)
			{
				ff_log_debug(L"end of file reached while reading the file=%p, buf=%p, len=%d", file, buf, len);
				int_bytes_read = 0;
			}
			else
			{
				ff_log_debug(L"unexpected error occured while reading the file=%p, buf=%p, len=%d. GetLastError()=%lu", file, buf, len, last_error);
			}
			goto end;
		}
	}

	int_bytes_read = complete_overlapped_io(file, &overlapped);
	if (int_bytes_read == -1)
	{
		ff_log_debug(L"cannot read %d bytes from the file=%p to the buf=%p using overlapped=%p. See previous messages for more info", len, file, buf, &overlapped);
	}

end:
	return int_bytes_read;
}

int ff_arch_file_write(struct ff_arch_file *file, const void *buf, int len)
{
	OVERLAPPED overlapped;
	BOOL result;
	DWORD bytes_written;
	int int_bytes_written = -1;

	ff_assert(len >= 0);

	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.Offset = (DWORD) file->curr_pos;
	overlapped.OffsetHigh = (DWORD) (file->curr_pos >> 32);
	result = WriteFile(file->handle, buf, (DWORD) len, &bytes_written, &overlapped);
	if (result == FALSE)
	{
		DWORD last_error;

		last_error = GetLastError();
		if (last_error != ERROR_IO_PENDING)
		{
			ff_log_debug(L"unexpected error occured while writing to the file=%p, buf=%p, len=%d. GetLastError()=%lu", file, buf, len, last_error);
			goto end;
		}
	}

	int_bytes_written = complete_overlapped_io(file, &overlapped);
	if (int_bytes_written == -1)
	{
		ff_log_debug(L"cannot write %d bytes to the file=%p from the buf=%p using overlapped=%p. See previous messages for more info", len, file, buf, &overlapped);
	}

end:
	return int_bytes_written;
}

enum ff_result ff_arch_file_erase(const wchar_t *path)
{
	struct threadpool_erase_file_data data;

	data.path = path;
	data.result = FF_FAILURE;
	ff_core_threadpool_execute(threadpool_erase_file_func, &data);
	if (data.result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot erase the file [%ls]. See previous messages for more info", path);
	}

	return data.result;
}

enum ff_result ff_arch_file_copy(const wchar_t *src_path, const wchar_t *dst_path)
{
	struct threadpool_copy_file_data data;

	data.src_path = src_path;
	data.dst_path = dst_path;
	data.result = FF_FAILURE;
	ff_core_threadpool_execute(threadpool_copy_file_func, &data);
	if (data.result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot copy the file [%ls] to the [%ls]. See previous messages for more info", src_path, dst_path);
	}

	return data.result;
}

enum ff_result ff_arch_file_move(const wchar_t *src_path, const wchar_t *dst_path)
{
	struct threadpool_move_file_data data;

	data.src_path = src_path;
	data.dst_path = dst_path;
	data.result = FF_FAILURE;
	ff_core_threadpool_execute(threadpool_move_file_func, &data);
	if (data.result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot move the file [%ls] to the [%ls]. See previous messages for more info", src_path, dst_path);
	}

	return data.result;
}

int64_t ff_arch_file_get_size(struct ff_arch_file *file)
{
	LARGE_INTEGER size;
	BOOL result;
	int64_t file_size;

	result = GetFileSizeEx(file->handle, &size);
	ff_winapi_fatal_error_check(result != FALSE, L"cannot determine file size");

	file_size = (int64_t) size.QuadPart;
	return file_size;
}
