#include "private/ff_common.h"

#include "private/ff_file.h"
#include "private/arch/ff_arch_file.h"
#include "private/ff_read_stream_buffer.h"
#include "private/ff_write_stream_buffer.h"

static const int BUFFER_SIZE = 0x10000;

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
	return bytes_read;
}

static int file_write_func(void *ctx, const void *buf, int len)
{
	struct ff_file *file;
	int bytes_written;

	ff_assert(len >= 0);

	file = (struct ff_file *) ctx;
	bytes_written = ff_arch_file_write(file->file, buf, len);
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

int ff_file_read(struct ff_file *file, void *buf, int len)
{
	int bytes_read;

	ff_assert(file->access_mode == FF_FILE_READ);
	ff_assert(len >= 0);

	bytes_read = ff_read_stream_buffer_read(file->buffers.read_buffer, buf, len);
	return bytes_read;
}

int ff_file_write(struct ff_file *file, const void *buf, int len)
{
	int bytes_written;

	ff_assert(file->access_mode == FF_FILE_WRITE);
	ff_assert(len >= 0);

	bytes_written = ff_write_stream_buffer_write(file->buffers.write_buffer, buf, len);
	return bytes_written;
}

int ff_file_flush(struct ff_file *file)
{
	int bytes_written;

	ff_assert(file->access_mode == FF_FILE_WRITE);
	bytes_written = ff_write_stream_buffer_flush(file->buffers.write_buffer);
	return bytes_written;
}

int ff_file_erase(const wchar_t *path)
{
	int is_success;

	is_success = ff_arch_file_erase(path);
	return is_success;
}

int ff_file_copy(const wchar_t *src_path, const wchar_t *dst_path)
{
	int is_success;

	is_success = ff_arch_file_copy(src_path, dst_path);
	return is_success;
}

int ff_file_move(const wchar_t *src_path, const wchar_t *dst_path)
{
	int is_success;

	is_success = ff_arch_file_move(src_path, dst_path);
	return is_success;
}

int64_t ff_file_get_size(struct ff_file *file)
{
	int64_t file_size;

	file_size = ff_arch_file_get_size(file->file);
	return file_size;
}
