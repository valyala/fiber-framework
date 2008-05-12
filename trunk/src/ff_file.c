#include "private/ff_common.h"

#include "private/ff_file.h"
#include "private/arch/ff_arch_file.h"
#include "private/ff_read_stream_buffer.h"
#include "private/ff_write_stream_buffer.h"

static const int READ_BUFFER_SIZE = 0x10000;
static const int WRITE_BUFFER_SIZE = 0x10000;

struct ff_file
{
	struct ff_arch_file *file;
	struct ff_read_stream_buffer *read_buffer;
	struct ff_write_stream_buffer *write_buffer;
};

static int read_func(void *ctx, void *buf, int len)
{
	struct ff_file *file;
	int bytes_read;

	file = (struct ff_file *) ctx;
	bytes_read = ff_arch_file_read(file->file, buf, len);
	return bytes_read;
}

static int write_func(void *ctx, const void *buf, int len)
{
	struct ff_file *file;
	int bytes_written;

	file = (struct ff_file *) ctx;
	bytes_written = ff_arch_file_write(file->file, buf, len);
	return bytes_written;
}

struct ff_file *ff_file_open(const wchar_t *path, enum ff_file_access_mode access_mode)
{
	struct ff_file *file;
	enum ff_arch_file_access_mode arch_access_mode;

	arch_access_mode = access_mode == FF_FILE_READ ? FF_ARCH_FILE_READ : FF_ARCH_FILE_WRITE;

	file = (struct ff_file *) ff_malloc(sizeof(*file));
	file->file = ff_arch_file_open(path, arch_access_mode);
	file->read_buffer = ff_read_stream_buffer_create(read_func, file, READ_BUFFER_SIZE);
	file->write_buffer = ff_write_stream_buffer_create(write_func, file, WRITE_BUFFER_SIZE);

	return file;
}

void ff_file_close(struct ff_file *file)
{
	ff_write_stream_buffer_delete(file->write_buffer);
	ff_read_stream_buffer_delete(file->read_buffer);
	ff_arch_file_close(file->file);
	ff_free(file);
}

int ff_file_read(struct ff_file *file, void *buf, int len)
{
	int bytes_read;

	bytes_read = ff_read_stream_buffer_read(file->read_buffer, buf, len);
	return bytes_read;
}

int ff_file_write(struct ff_file *file, const void *buf, int len)
{
	int bytes_written;

	bytes_written = ff_write_stream_buffer_write(file->write_buffer, buf, len);
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
