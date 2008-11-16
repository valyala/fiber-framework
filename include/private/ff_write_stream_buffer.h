#ifndef FF_WRITE_STREAM_BUFFER_PRIVATE
#define FF_WRITE_STREAM_BUFFER_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ff_write_stream_func)(void *ctx, const void *buf, int len);

struct ff_write_stream_buffer;

/**
 * Creates the buffer for writing.
 * write_func will be used for writing data into underlying stream in the case
 * when the buffer will be full.
 * write_func_ctx is the context parameter, which is passed to the write_func. Usually it points to
 * the underlying stream, to which the write_func will write data.
 * capacity is the size of the buffer in bytes.
 */
struct ff_write_stream_buffer *ff_write_stream_buffer_create(ff_write_stream_func write_func, void *write_func_ctx, int capacity);

/**
 * Deletes the buffer.
 */
void ff_write_stream_buffer_delete(struct ff_write_stream_buffer *buffer);

/**
 * Writes exactly len bytes to the buffer.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
enum ff_result ff_write_stream_buffer_write(struct ff_write_stream_buffer *buffer, const void *buf, int len);

/**
 * Flushes the buffer.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
enum ff_result ff_write_stream_buffer_flush(struct ff_write_stream_buffer *buffer);

#ifdef __cplusplus
}
#endif

#endif
