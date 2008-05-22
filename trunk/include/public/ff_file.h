#ifndef FF_FILE_PUBLIC
#define FF_FILE_PUBLIC

#include "public/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_file;

enum ff_file_access_mode
{
	FF_FILE_READ,
	FF_FILE_WRITE
};

FF_API struct ff_file *ff_file_open(const wchar_t *path, enum ff_file_access_mode access_mode);

FF_API void ff_file_close(struct ff_file *file);

FF_API int ff_file_read(struct ff_file *file, void *buf, int len);

FF_API int ff_file_write(struct ff_file *file, const void *buf, int len);

FF_API int ff_file_flush(struct ff_file *file);

FF_API int ff_file_erase(const wchar_t *path);

FF_API int ff_file_copy(const wchar_t *src_path, const wchar_t *dst_path);

FF_API int ff_file_move(const wchar_t *src_path, const wchar_t *dst_path);

FF_API int64_t ff_file_get_size(struct ff_file *file);

#ifdef __cplusplus
}
#endif

#endif
