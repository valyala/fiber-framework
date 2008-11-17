#ifndef FF_ARCH_MISC_PUBLIC_H
#define FF_ARCH_MISC_PUBLIC_H

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * returns path to temporary directory and its length.
 * Path to temporary directory has '/' or '\' character at the end.
 */
FF_API void ff_arch_misc_get_tmp_dir_path(const wchar_t **tmp_dir_path, int *tmp_dir_path_len);

/**
 * creates string representation of GUID.
 * The caller should call ff_arch_misc_delete_guid_cstr() in order to free resources
 * allocated for the guid_cstr.
 */
FF_API void ff_arch_misc_create_guid_cstr(const wchar_t **guid_cstr, int *guid_cstr_len);

/**
 * Frees resources allocated for the guid_cstr by the ff_arch_misc_create_guid_cstr()
 */
FF_API void ff_arch_misc_delete_guid_cstr(const wchar_t *guid_cstr);

/**
 * creates unique file path using the following template:
 * {dir_path}{prefix}{unique_string}
 * dir_path should have '/' or '\' character at the end.
 * prefix can be NULL only if the prefix_len is equal to zero.
 * The caller should call the ff_arch_misc_delete_unique_file_path() in order to
 * free resources allocated for the unique_file_path.
 */
FF_API void ff_arch_misc_create_unique_file_path(const wchar_t *dir_path, int dir_path_len,
	const wchar_t *prefix, int prefix_len, const wchar_t **unique_file_path, int *unique_file_path_len);

/**
 * Deletes the unique_file_path allocated unsing ff_arch_misc_create_unique_file_path.
 */
FF_API void ff_arch_misc_delete_unique_file_path(const wchar_t *unique_file_path);

/**
 * fills the given buffer with the given buf_len length by (pseudo)random data.
 */
FF_API void ff_arch_misc_fill_buffer_with_random_data(void *buf, int buf_len);

#ifdef __cplusplus
}
#endif

#endif
