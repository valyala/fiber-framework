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
	 * It should return 1 on success, 0 on error.
	 */
	int (*read)(struct ff_stream *stream, void *buf, int len);

	/**
	 * the read_with_timeout() callback should read exactly len bytes from the stream into buf.
	 * It should return 0 if the data cannot be read during the given timeout.
	 * It should return 1 on success, 0 on error.
	 */
	int (*read_with_timeout)(struct ff_stream *stream, void *buf, int len, int timeout);

	/**
	 * the write() callback should write exactly len bytes from the buf into the stream.
	 * It should return 1 on success, 0 on error.
	 */
	int (*write)(struct ff_stream *stream, const void *buf, int len);

	/**
	 * the write_with_timeout() callback should write exactly len bytes from the buf into the stream.
	 * It should return 0 if the data cannot be written during the given timeout.
	 * It should return 1 on success, 0 on error.
	 */
	int (*write_with_timeout)(struct ff_stream *stream, const void *buf, int len, int timeout);

	/**
	 * the flush() callback should flush stream's write buffer.
	 * It should return 1 on success, 0 on error.
	 */
	int (*flush)(struct ff_stream *stream);

	/**
	 * the disconnect() callback should unblock all pending write*() and read*() calls.
	 * All subsequent write*() and read*() calls should return 0 (error) immediately.
	 */
	void (*disconnect)(struct ff_stream *stream);
};

/**
 * Creates a stream using the given vtable and ctx.
 * ctx then can be obtained by ff_stream_get_ctx() function.
 */
FF_API struct ff_stream *ff_stream_create(struct ff_stream_vtable *vtable, void *ctx);

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
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_stream_read(struct ff_stream *stream, void *buf, int len);

/**
 * Reads exactly len bytes from the stream into the buf.
 * Returns 0 if the data cannot be read during the given timeout milliseconds.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_stream_read_with_timeout(struct ff_stream *stream, void *buf, int len, int timeout);

/**
 * Writes exactly len bytes from the buf into the stream.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_stream_write(struct ff_stream *stream, const void *buf, int len);

/**
 * Writes exactly len bytes from the buf into the stream.
 * Returns 0 of the data cannot be written during the given timeout milliseconds.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_stream_write_with_timeout(struct ff_stream *stream, const void *buf, int len, int timeout);

/**
 * Flushes the stream's write buffer.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_stream_flush(struct ff_stream *stream);

/**
 * Disconnects the stream.
 * It unblocks all ff_stream_read*() and ff_stream_write*() pending calls.
 * Subsequent calls to the ff_stream_read*() and ff_stream_write*() will return 0 (error) immediately.
 */
FF_API void ff_stream_disconnect(struct ff_stream *stream);

#ifdef __cplusplus
}
#endif

#endif
