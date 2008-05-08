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

end:
	return file;
}

void ff_arch_file_close(struct ff_arch_file *file)
{
	BOOL result;

	result = CloseHandle(file->handle);
	ff_assert(result != FALSE);
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
