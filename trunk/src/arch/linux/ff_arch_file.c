#include "private/ff_common.h"

#include "private/arch/ff_arch_file.h"
#include "private/ff_core.h"
#include "private/ff_fiber.h"
#include "ff_linux_file.h"
#include "ff_linux_completion_port.h"
#include "ff_linux_error_check.h"
#include "ff_linux_misc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_COPY_BUF_SIZE 0x10000

struct ff_arch_file
{
	int fd;
	enum ff_arch_file_access_mode access_mode;
};

struct threadpool_open_file_data
{
	const char *path;
	enum ff_arch_file_access_mode access_mode;
	int fd;
};

struct threadpool_erase_file_data
{
	const char *path;
	int is_success;
};

struct threadpool_copy_file_data
{
	const char *src_path;
	const char *dst_path;
	int is_success;
};

struct threadpool_move_file_data
{
	const char *src_path;
	const char *dst_path;
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
	int flags;
	int rv;

	data = (struct threadpool_open_file_data *) ctx;
	flags = ((data->access_mode == FF_ARCH_FILE_READ) ? O_RDONLY : (O_WRONLY | O_CREAT | O_TRUNC));
	flags |= O_LARGEFILE;
	data->fd = open(data->path, flags, S_IREAD | S_IWRITE);
	if (data->fd != -1)
	{
		rv = fcntl(data->fd, F_SETFL, O_NONBLOCK);
		ff_linux_fatal_error_check(rv != -1, L"cannot set nonblocking mode for the file descriptor");
	}
}

static void threadpool_erase_file_func(void *ctx)
{
	struct threadpool_erase_file_data *data;
	int rv;

	data = (struct threadpool_erase_file_data *) ctx;
	rv = unlink(data->path);
	data->is_success = (rv == -1) ? 0 : 1;
}

static void threadpool_copy_file_func(void *ctx)
{
	struct threadpool_copy_file_data *data;
	int rv;
	int src_fd, dst_fd;
	char *buf;

	data = (struct threadpool_copy_file_data *) ctx;
	data->is_success = 0;
	src_fd = open(data->src_path, O_RDONLY | O_LARGEFILE);
	if (src_fd == -1)
	{
		return;
	}
	dst_fd = open(data->dst_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (dst_fd == -1)
	{
		rv = close(src_fd);
		ff_assert(rv != -1);
		return;
	}

	buf = (char *) ff_malloc(FILE_COPY_BUF_SIZE);
	for (;;)
	{
		ssize_t bytes_read, bytes_written;

		for (;;)
		{
			bytes_read = read(src_fd, buf, FILE_COPY_BUF_SIZE);
			if (bytes_read != -1 || errno != EINTR)
			{
				break;
			}
		}
		if (bytes_read == 0)
		{
			data->is_success = 1;
			break;
		}
		if (bytes_read == -1)
		{
			break;
		}
		ff_assert(bytes_read > 0);
		while (bytes_read > 0)
		{
			for (;;)
			{
				bytes_written = write(dst_fd, buf, bytes_read);
				if (bytes_written != -1 || errno != EINTR)
				{
					break;
				}
			}
			if (bytes_written == -1)
			{
				break;
			}
			ff_assert(bytes_written > 0);
			bytes_read -= bytes_written;
		}
		if (bytes_written == -1)
		{
			break;
		}
		ff_assert(bytes_read == 0);
	}
	ff_free(buf);

	rv = close(dst_fd);
	ff_assert(rv != -1);
	close(src_fd);
	ff_assert(rv != -1);
}

static void threadpool_move_file_func(void *ctx)
{
	struct threadpool_move_file_data *data;
	int rv;

	data = (struct threadpool_move_file_data *) ctx;
	rv = rename(data->src_path, data->dst_path);
	data->is_success = (rv == -1) ? 0 : 1;
}

static void wait_for_file_io(struct ff_arch_file *file)
{
	struct ff_fiber *current_fiber;
	enum ff_linux_completion_port_operation_type operation_type;

	current_fiber = ff_fiber_get_current();
	operation_type = (file->access_mode == FF_ARCH_FILE_READ) ? FF_COMPLETION_PORT_OPERATION_READ : FF_COMPLETION_PORT_OPERATION_WRITE;
	ff_linux_completion_port_register_operation(file_ctx.completion_port, file->fd, operation_type, current_fiber);
	ff_core_yield_fiber();
}

void ff_linux_file_initialize(struct ff_arch_completion_port *completion_port)
{
	file_ctx.completion_port = completion_port;
}

void ff_linux_file_shutdown()
{
	/* nothing to do */
}

struct ff_arch_file *ff_arch_file_open(const wchar_t *path, enum ff_arch_file_access_mode access_mode)
{
	struct ff_arch_file *file = NULL;
	char *mb_path;
	struct threadpool_open_file_data data;

	mb_path = ff_linux_misc_wide_to_multibyte_string(path);
	data.path = mb_path;
	data.access_mode = access_mode;
	data.fd = -1;
	ff_core_threadpool_execute(threadpool_open_file_func, &data);
	ff_free(mb_path);
	if (data.fd != -1)
	{
		file = (struct ff_arch_file *) ff_malloc(sizeof(*file));
		file->fd = data.fd;
		file->access_mode = access_mode;
	}

	return file;
}

void ff_arch_file_close(struct ff_arch_file *file)
{
	int rv;

	rv = close(file->fd);
	ff_assert(rv != -1);
	ff_free(file);
}

int ff_arch_file_read(struct ff_arch_file *file, void *buf, int len)
{
	ssize_t bytes_read;
	int bytes_read_int;

	ff_assert(len > 0);
	ff_assert(file->access_mode == FF_ARCH_FILE_READ);

again:
	bytes_read = read(file->fd, buf, len);
	if (bytes_read == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN)
		{
			wait_for_file_io(file);
			goto again;
		}
	}

	bytes_read_int = (int) bytes_read;
	return bytes_read_int;
}

int ff_arch_file_write(struct ff_arch_file *file, const void *buf, int len)
{
	ssize_t bytes_written;
	int bytes_written_int;

	ff_assert(len > 0);
	ff_assert(file->access_mode == FF_ARCH_FILE_WRITE);

again:
	bytes_written = write(file->fd, buf, len);
	if (bytes_written == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN)
		{
			wait_for_file_io(file);
			goto again;
		}
	}

	bytes_written_int = (int) bytes_written;
	return bytes_written;
}

int ff_arch_file_erase(const wchar_t *path)
{
	char *mb_path;
	struct threadpool_erase_file_data data;

	mb_path = ff_linux_misc_wide_to_multibyte_string(path);
	data.path = mb_path;
	data.is_success = 0;
	ff_core_threadpool_execute(threadpool_erase_file_func, &data);
	ff_free(mb_path);

	return data.is_success;
}

int ff_arch_file_copy(const wchar_t *src_path, const wchar_t *dst_path)
{
	char *mb_src_path;
	char *mb_dst_path;
	struct threadpool_copy_file_data data;

	mb_src_path = ff_linux_misc_wide_to_multibyte_string(src_path);
	mb_dst_path = ff_linux_misc_wide_to_multibyte_string(dst_path);
	data.src_path = mb_src_path;
	data.dst_path = mb_dst_path;
	data.is_success = 0;
	ff_core_threadpool_execute(threadpool_copy_file_func, &data);
	ff_free(mb_src_path);
	ff_free(mb_dst_path);

	return data.is_success;
}

int ff_arch_file_move(const wchar_t *src_path, const wchar_t *dst_path)
{
	char *mb_src_path;
	char *mb_dst_path;
	struct threadpool_move_file_data data;

	mb_src_path = ff_linux_misc_wide_to_multibyte_string(src_path);
	mb_dst_path = ff_linux_misc_wide_to_multibyte_string(dst_path);
	data.src_path = mb_src_path;
	data.dst_path = mb_dst_path;
	data.is_success = 0;
	ff_core_threadpool_execute(threadpool_move_file_func, &data);
	ff_free(mb_src_path);
	ff_free(mb_dst_path);

	return data.is_success;
}

int64_t ff_arch_file_get_size(struct ff_arch_file *file)
{
	struct stat stat;
	int rv;
	int64_t size;

	rv = fstat(file->fd, &stat);
	ff_linux_fatal_error_check(rv != -1, L"error in the stat() function");
	size = (int64_t) stat.st_size;
	ff_assert(size >= 0);

	return size;
}
