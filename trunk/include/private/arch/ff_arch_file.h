#ifndef FF_ARCH_FILE_PRIVATE
#define FF_ARCH_FILE_PRIVATE

#include "private/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_file;

enum ff_arch_file_access_mode
{
	FF_ARCH_FILE_READ,
	FF_ARCH_FILE_WRITE
};

struct ff_arch_file *ff_arch_file_open(const wchar_t *path, enum ff_arch_file_access_mode access_mode);

void ff_arch_file_close(struct ff_arch_file *file);

int ff_arch_file_read(struct ff_arch_file *file, void *buf, int len);

int ff_arch_file_write(struct ff_arch_file *file, const void *buf, int len);

int ff_arch_file_erase(const wchar_t *path);

int ff_arch_file_copy(const wchar_t *src_path, const wchar_t *dst_path);

int ff_arch_file_move(const wchar_t *src_path, const wchar_t *dst_path);

int64_t ff_arch_file_get_size(struct ff_arch_file *file);

#ifdef __cplusplus
}
#endif

#endif
