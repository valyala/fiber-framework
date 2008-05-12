#ifndef FF_WRITE_STREAM_BUFFER_PRIVATE
#define FF_WRITE_STREAM_BUFFER_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ff_write_stream_func)(void *ctx, const void *buf, int len);

struct ff_write_stream_buffer;

struct ff_write_stream_buffer *ff_write_stream_buffer_create(ff_write_stream_func write_func, void *func_ctx, int capacity);

void ff_write_stream_buffer_delete(struct ff_write_stream_buffer *buffer);

int ff_write_stream_buffer_write(struct ff_write_stream_buffer *buffer, const void *buf, int len);

int ff_write_stream_buffer_flush(struct ff_write_stream_buffer *buffer);

#ifdef __cplusplus
}
#endif

#endif
