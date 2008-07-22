#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_file.h"
#include "private/ff_core.h"
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
	int is_success;
};

struct threadpool_copy_file_data
{
	const wchar_t *src_path;
	const wchar_t *dst_path;
	int is_success;
};

struct threadpool_move_file_data
{
	const wchar_t *src_path;
	const wchar_t *dst_path;
	int is_success;
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
	DWORD creation_disposition;

	data = (struct threadpool_open_file_data *) ctx;
	access_rights = (data->access_mode == FF_ARCH_FILE_READ) ? GENERIC_READ : GENERIC_WRITE;
	creation_disposition = (data->access_mode == FF_ARCH_FILE_READ) ? OPEN_EXISTING : CREATE_ALWAYS;
	data->handle = CreateFileW(data->path, access_rights, 0, NULL, creation_disposition,
		FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
}

static void threadpool_erase_file_func(void *ctx)
{
	struct threadpool_erase_file_data *data;
	BOOL result;

	data = (struct threadpool_erase_file_data *) ctx;
	result = DeleteFileW(data->path);
	data->is_success = (result == FALSE) ? 0 : 1;
}

static void threadpool_copy_file_func(void *ctx)
{
	struct threadpool_copy_file_data *data;
	BOOL result;

	data = (struct threadpool_copy_file_data *) ctx;
	result = CopyFileW(data->src_path, data->dst_path, TRUE);
	data->is_success = (result == FALSE) ? 0 : 1;
}

static void threadpool_move_file_func(void *ctx)
{
	struct threadpool_move_file_data *data;
	BOOL result;

	data = (struct threadpool_move_file_data *) ctx;
	result = MoveFileEx(data->src_path, data->dst_path, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	data->is_success = (result == FALSE) ? 0 : 1;
}

static int complete_overlapped_io(struct ff_arch_file *file, OVERLAPPED *overlapped)
{
	struct ff_fiber *current_fiber;
	int int_bytes_transferred = -1;
	BOOL result;
	DWORD bytes_transferred;

	current_fiber = ff_core_get_current_fiber();
	ff_win_completion_port_register_overlapped_data(file_ctx.completion_port, overlapped, current_fiber);
	ff_core_yield_fiber();
	ff_win_completion_port_deregister_overlapped_data(file_ctx.completion_port, overlapped);

	result = GetOverlappedResult(file->handle, overlapped, &bytes_transferred, FALSE);
	if (result != FALSE)
	{
		int_bytes_transferred = (int) bytes_transferred;
		file->curr_pos += int_bytes_transferred;
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
				int_bytes_read = 0;
			}
			goto end;
		}
	}

	int_bytes_read = complete_overlapped_io(file, &overlapped);

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
			goto end;
		}
	}

	int_bytes_written = complete_overlapped_io(file, &overlapped);

end:
	return int_bytes_written;
}

int ff_arch_file_erase(const wchar_t *path)
{
	struct threadpool_erase_file_data data;

	data.path = path;
	data.is_success = 0;
	ff_core_threadpool_execute(threadpool_erase_file_func, &data);

	return data.is_success;
}

int ff_arch_file_copy(const wchar_t *src_path, const wchar_t *dst_path)
{
	struct threadpool_copy_file_data data;

	data.src_path = src_path;
	data.dst_path = dst_path;
	data.is_success = 0;
	ff_core_threadpool_execute(threadpool_copy_file_func, &data);

	return data.is_success;
}

int ff_arch_file_move(const wchar_t *src_path, const wchar_t *dst_path)
{
	struct threadpool_move_file_data data;

	data.src_path = src_path;
	data.dst_path = dst_path;
	data.is_success = 0;
	ff_core_threadpool_execute(threadpool_move_file_func, &data);

	return data.is_success;
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
