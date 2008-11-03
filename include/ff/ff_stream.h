#ifndef FF_STREAM_PUBLIC
#define FF_STREAM_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_stream;

struct ff_stream_vtable
{
	/**
	 * the delete() callback should release the context passed to the ff_stream_create()
	 */
	void (*delete)(struct ff_stream *stream);

	/**
	 * the read() callback should read exactly len bytes from the stream into buf.
	 * It should return FF_SUCCESS on success, FF_FAILURE on error.
	 */
	enum ff_result (*read)(struct ff_stream *stream, void *buf, int len);

	/**
	 * the write() callback should write exactly len bytes from the buf into the stream.
	 * It should return FF_SUCCESS on success, FF_FAILURE on error.
	 */
	enum ff_result (*write)(struct ff_stream *stream, const void *buf, int len);

	/**
	 * the flush() callback should flush stream's write buffer.
	 * It should return FF_SUCCESS on success, FF_FAILURE on error.
	 */
	enum ff_result (*flush)(struct ff_stream *stream);

	/**
	 * the disconnect() callback should unblock all pending write*() and read*() calls.
	 * All subsequent write*() and read*() calls should return FF_FAILURE immediately.
	 */
	void (*disconnect)(struct ff_stream *stream);
};

/**
 * Creates a stream using the given vtable and ctx.
 * ctx then can be obtained by ff_stream_get_ctx() function.
 */
FF_API struct ff_stream *ff_stream_create(const struct ff_stream_vtable *vtable, void *ctx);

/**
 * Deletes the stream.
 * It invokes the ff_stream_vtable::delete() callback.
 */
FF_API void ff_stream_delete(struct ff_stream *stream);

/**
 * Returns the context parameter, which was passed to the ff_stream_create()
 */
FF_API void *ff_stream_get_ctx(struct ff_stream *stream);

/**
 * Reads exactly len bytes from the stream into the buf.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
FF_API enum ff_result ff_stream_read(struct ff_stream *stream, void *buf, int len);

/**
 * Writes exactly len bytes from the buf into the stream.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
FF_API enum ff_result ff_stream_write(struct ff_stream *stream, const void *buf, int len);

/**
 * Flushes the stream's write buffer.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
FF_API enum ff_result ff_stream_flush(struct ff_stream *stream);

/**
 * Disconnects the stream.
 * It unblocks all ff_stream_read*() and ff_stream_write*() pending calls.
 * Subsequent calls to the ff_stream_read*() and ff_stream_write*() will return FF_FAILURE immediately.
 */
FF_API void ff_stream_disconnect(struct ff_stream *stream);

/**
 * Copies exactly len bytes from the src_stream to the dst_stream.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
FF_API enum ff_result ff_stream_copy(struct ff_stream *src_stream, struct ff_stream *dst_stream, int len);

/**
 * Calculates hash value for len bytes from the stream using start_value as the hash seed.
 * Stores the hash value in the hash_value.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
FF_API enum ff_result ff_stream_get_hash(struct ff_stream *stream, int len, uint32_t start_value, uint32_t *hash_value);

#ifdef __cplusplus
}
#endif

#endif
