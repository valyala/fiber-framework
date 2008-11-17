#ifndef FF_STREAM_ACCEPTOR_PUBLIC_H
#define FF_STREAM_ACCEPTOR_PUBLIC_H

#include "ff/ff_common.h"
#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_stream_acceptor;

struct ff_stream_acceptor_vtable
{
	/**
	 * the delete() callback should release the context passed to the ff_stream_acceptor_create()
	 */
	void (*delete)(struct ff_stream_acceptor *stream_acceptor);

	/**
	 * the initialize() callback should initialize the stream_acceptor for subsequent usage
	 * of the ff_stream_acceptor_accept() method.
	 * This method is called before the first call into the accept() callback
	 * and after the shutdown() call.
	 * The callback must call ff_log_fatal_error() if it cannot finish initialization.
	 */
	void (*initialize)(struct ff_stream_acceptor *stream_acceptor);

	/**
	 * the shutdown() callback should unblock the currently blocked ff_stream_acceptor_accept()
	 * functions. These functions should return NULL and subsequent calls to the ff_stream_acceptor_accept()
	 * should immediately return NULL without blocking.
	 * Subsequent call to the initialize() callback should restore ff_stream_acceptor_accept()
	 * functionality.
	 */
	void (*shutdown)(struct ff_stream_acceptor *stream_acceptor);

	/**
	 * the accept() callback should return next accepted stream or NULL on error
	 * or after the ff_stream_acceptor_shutdown() was called or if the ff_stream_acceptor_initialize()
	 * wasn't called before.
	 */
	struct ff_stream *(*accept)(struct ff_stream_acceptor *stream_acceptor);
};

/**
 * Creates a stream_acceptor using given vtable and ctx.
 * ctx then can be obtained by ff_stream_acceptor_get_ctx() function.
 * vtable must be persistent until the ff_stream_acceptor_delete() will be called.
 * Always returns correct result.
 */
FF_API struct ff_stream_acceptor *ff_stream_acceptor_create(const struct ff_stream_acceptor_vtable *vtable, void *ctx);

/**
 * Deletes the given stream_acceptor.
 */
FF_API void ff_stream_acceptor_delete(struct ff_stream_acceptor *stream_acceptor);

/**
 * Returns context passed to the ff_stream_acceptor_create().
 */
FF_API void *ff_stream_acceptor_get_ctx(struct ff_stream_acceptor *stream_acceptor);

/**
 * Initializes the stream_acceptor.
 * This function should be called before calling into the ff_stream_acceptor_accept()
 * and after the ff_stream_acceptor_shutdown() was called.
 */
FF_API void ff_stream_acceptor_initialize(struct ff_stream_acceptor *stream_acceptor);

/**
 * Shutdowns the stream_acceptor. Currently blocked ff_stream_acceptor_accept() call
 * immediately unblocks and returns NULL. Subsequent calls to the ff_stream_acceptor_accept()
 * immediately return NULL without blocking.
 * The stream_acceptor can be restored by subsequent call into the ff_stream_acceptor_initialize(),
 * so the ff_stream_acceptor_accept() will work again.
 */
FF_API void ff_stream_acceptor_shutdown(struct ff_stream_acceptor *stream_acceptor);

/**
 * Accepts the next stream from the stream_acceptor.
 * Returns accepted stream on success or NULL on error
 * or after the ff_stream_acceptor_disconnect() was called.
 */
FF_API struct ff_stream *ff_stream_acceptor_accept(struct ff_stream_acceptor *stream_acceptor);

#ifdef __cplusplus
}
#endif

#endif
