#ifndef FF_READ_STREAM_BUFFER_PRIVATE
#define FF_READ_STREAM_BUFFER_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ff_read_stream_func)(void *ctx, void *buf, int len);

struct ff_read_stream_buffer;

struct ff_read_stream_buffer *ff_read_stream_buffer_create(ff_read_stream_func read_func, void *func_ctx, int capacity);

void ff_read_stream_buffer_delete(struct ff_read_stream_buffer *buffer);

int ff_read_stream_buffer_read(struct ff_read_stream_buffer *buffer, void *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
