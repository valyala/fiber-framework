#include "private/arch/ff_arch_file.h"
#include "private/ff_core.h"

struct ff_arch_file
{
	int fd;
};

struct file_data
{
	struct ff_arch_completion_port *completion_port;
};

static struct file_data file_ctx;

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
}

void ff_arch_file_close(struct ff_arch_file *file)
{
}

int ff_arch_file_read(struct ff_arch_file *file, void *buf, int len)
{
}

int ff_arch_file_write(struct ff_arch_file *file, const void *buf, int len)
{
}

int ff_arch_file_erase(const wchar_t *path)
{
}

int ff_arch_file_copy(const wchar_t *src_path, const wchar_t *dst_path)
{
}

int ff_arch_file_move(const wchar_t *src_path, const wchar_t *dst_path)
{
}

int64_t ff_arch_file_get_size(struct ff_arch_file *file)
{
}
