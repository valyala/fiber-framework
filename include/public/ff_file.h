#ifndef FF_FILE_PUBLIC
#define FF_FILE_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

struct ff_file;

enum ff_file_access_mode
{
	FF_FILE_READ,
	FF_FILE_WRITE
};

struct ff_file *ff_file_open(const wchar_t *path, enum ff_file_access_mode access_mode);

void ff_file_close(struct ff_file *file);

int ff_file_read(struct ff_file *file, void *buf, int len);

int ff_file_write(struct ff_file *file, const void *buf, int len);

int ff_file_erase(const wchar_t *path);

int ff_file_copy(const wchar_t *src_path, const wchar_t *dst_path);

int ff_file_move(const wchar_t *src_path, const wchar_t *dst_path);

int64_t ff_file_get_size(struct ff_file *file);

#ifdef __cplusplus
}
#endif

#endif
