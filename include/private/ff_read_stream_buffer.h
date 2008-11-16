#ifndef FF_READ_STREAM_BUFFER_PRIVATE
#define FF_READ_STREAM_BUFFER_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ff_read_stream_func)(void *ctx, void *buf, int len);

struct ff_read_stream_buffer;

/**
 * Creates a buffer for reading.
 * read_func is the function, which will be called for reading the next chunk of data
 * in the case if the buffer become empty.
 * read_func_ctx is the context parameter, which will be passed to the read_func. Usually this parameter
 * points to the underlying stream, from which the read_func will read data.
 * capacity is size of the buffer in bytes.
 */
struct ff_read_stream_buffer *ff_read_stream_buffer_create(ff_read_stream_func read_func, void *read_func_ctx, int capacity);

/**
 * Deletes the buffer.
 */
void ff_read_stream_buffer_delete(struct ff_read_stream_buffer *buffer);

/**
 * Reads exactly len bytes from the buffer to the buf.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
enum ff_result ff_read_stream_buffer_read(struct ff_read_stream_buffer *buffer, void *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
