#include "private/ff_common.h"

#include "private/ff_file.h"
#include "private/arch/ff_arch_file.h"
#include "private/ff_read_stream_buffer.h"
#include "private/ff_write_stream_buffer.h"

#define BUFFER_SIZE 0x10000

struct ff_file
{
	struct ff_arch_file *file;
	union
	{
		struct ff_read_stream_buffer *read_buffer;
		struct ff_write_stream_buffer *write_buffer;
	} buffers;
	enum ff_file_access_mode access_mode;
};

static int file_read_func(void *ctx, void *buf, int len)
{
	struct ff_file *file;
	int bytes_read;

	ff_assert(len >= 0);

	file = (struct ff_file *) ctx;
	bytes_read = ff_arch_file_read(file->file, buf, len);
	if (bytes_read == -1)
	{
		ff_log_debug(L"error while reading from the file=%p to the buf=%p, len=%d. See previous messages for more info", file, buf, len);
	}
	return bytes_read;
}

static int file_write_func(void *ctx, const void *buf, int len)
{
	struct ff_file *file;
	int bytes_written;

	ff_assert(len >= 0);

	file = (struct ff_file *) ctx;
	bytes_written = ff_arch_file_write(file->file, buf, len);
	if (bytes_written == -1)
	{
		ff_log_debug(L"error while writing to the file=%p from the buf=%p, len=%d. See previous messages for more info", file, buf, len);
	}
	return bytes_written;
}

struct ff_file *ff_file_open(const wchar_t *path, enum ff_file_access_mode access_mode)
{
	struct ff_file *file;
	struct ff_arch_file *arch_file;
	enum ff_arch_file_access_mode arch_access_mode;

	file = NULL;
	arch_access_mode = access_mode == FF_FILE_READ ? FF_ARCH_FILE_READ : FF_ARCH_FILE_WRITE;
	arch_file = ff_arch_file_open(path, arch_access_mode);
	if (arch_file == NULL)
	{
		const char *mode;

		mode = (access_mode == FF_FILE_READ) ? "reading" : "writing";
		ff_log_debug(L"cannot open the file=[%ls] for %hs. See previous messages for more info", path, mode);
		goto end;
	}

	file = (struct ff_file *) ff_malloc(sizeof(*file));
	file->file = arch_file;
	file->access_mode = access_mode;
	if (access_mode == FF_FILE_READ)
	{
		file->buffers.read_buffer = ff_read_stream_buffer_create(file_read_func, file, BUFFER_SIZE);
	}
	else
	{
		file->buffers.write_buffer = ff_write_stream_buffer_create(file_write_func, file, BUFFER_SIZE);
	}

end:
	return file;
}

void ff_file_close(struct ff_file *file)
{
	if (file->access_mode == FF_FILE_READ)
	{
		ff_read_stream_buffer_delete(file->buffers.read_buffer);
	}
	else
	{
		ff_write_stream_buffer_delete(file->buffers.write_buffer);
	}
	ff_arch_file_close(file->file);
	ff_free(file);
}

enum ff_result ff_file_read(struct ff_file *file, void *buf, int len)
{
	enum ff_result result;

	ff_assert(file->access_mode == FF_FILE_READ);
	ff_assert(len >= 0);

	result = ff_read_stream_buffer_read(file->buffers.read_buffer, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while reading from the file=%p to the buf=%p, len=%d. See previous messages for more info", file, buf, len);
	}
	return result;
}

enum ff_result ff_file_write(struct ff_file *file, const void *buf, int len)
{
	enum ff_result result;

	ff_assert(file->access_mode == FF_FILE_WRITE);
	ff_assert(len >= 0);

	result = ff_write_stream_buffer_write(file->buffers.write_buffer, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while writing to the file=%p from the buf=%p, len=%d. See previous messages for more info", file, buf, len);
	}
	return result;
}

enum ff_result ff_file_flush(struct ff_file *file)
{
	enum ff_result result;

	ff_assert(file->access_mode == FF_FILE_WRITE);
	result = ff_write_stream_buffer_flush(file->buffers.write_buffer);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while flushing the file=%p. See previous messages for more info", file);
	}
	return result;
}

enum ff_result ff_file_erase(const wchar_t *path)
{
	enum ff_result result;

	result = ff_arch_file_erase(path);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while deleting the file=[%ls]. See previous messages for more info", path);
	}
	return result;
}

enum ff_result ff_file_copy(const wchar_t *src_path, const wchar_t *dst_path)
{
	enum ff_result result;

	result = ff_arch_file_copy(src_path, dst_path);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while copying the file=[%ls] to the [%ls]. See previous messages for more info", src_path, dst_path);
	}
	return result;
}

enum ff_result ff_file_move(const wchar_t *src_path, const wchar_t *dst_path)
{
	enum ff_result result;

	result = ff_arch_file_move(src_path, dst_path);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while moving the file=[%ls] to the [%ls]. See previous messages for more info", src_path, dst_path);
	}
	return result;
}

int64_t ff_file_get_size(struct ff_file *file)
{
	int64_t file_size;

	file_size = ff_arch_file_get_size(file->file);
	return file_size;
}
