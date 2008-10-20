#ifndef FF_FILE_PUBLIC
#define FF_FILE_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_file;

enum ff_file_access_mode
{
	FF_FILE_READ,
	FF_FILE_WRITE
};

/**
 * Opens the file on the given path in the given access_mode.
 * Returns opened file on success, NULL on error.
 */
FF_API struct ff_file *ff_file_open(const wchar_t *path, enum ff_file_access_mode access_mode);

/**
 * Closes the opened file.
 */
FF_API void ff_file_close(struct ff_file *file);

/**
 * Reads exaclty len bytes from the file into the buf.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_file_read(struct ff_file *file, void *buf, int len);

/**
 * Writes exaclty len bytes from the buf into the file.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_file_write(struct ff_file *file, const void *buf, int len);

/**
 * Flushes write buffer of the file.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_file_flush(struct ff_file *file);

/**
 * Erases the file on the given path.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_file_erase(const wchar_t *path);

/**
 * Copies the file from the src_path to the dst_path.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_file_copy(const wchar_t *src_path, const wchar_t *dst_path);

/**
 * Moves the file from the src_path to the dst_path.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_file_move(const wchar_t *src_path, const wchar_t *dst_path);

/**
 * Returns the size of the file.
 */
FF_API int64_t ff_file_get_size(struct ff_file *file);

#ifdef __cplusplus
}
#endif

#endif
