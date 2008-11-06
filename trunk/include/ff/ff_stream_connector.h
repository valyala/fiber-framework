#ifndef FF_STREAM_CONNECTOR_PUBLIC
#define FF_STREAM_CONNECTOR_PUBLIC

#include "ff/ff_common.h"
#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_stream_connector;

struct ff_stream_connector_vtable
{
	/**
	 * this callback must release resources allocated for the ctx, which was passed to the ff_stream_connector_create().
	 */
	void (*delete)(struct ff_stream_connector *stream_connector);

	/**
	 * the initialize() callback should initialize the stream_connector for subsequent usage
	 * of the ff_stream_connector_connect() method.
	 * This method is called before the first call into the connect() callback
	 * and after the shutdown() call.
	 * The callback must call ff_log_fatal_error() if it cannot finish initialization.
	 */
	void (*initialize)(struct ff_stream_connector *stream_connector);

	/**
	 * the shutdown() callback should unblock the currently blocked ff_stream_connector_connect()
	 * functions. These functions should return NULL and subsequent calls to the ff_stream_connector_connect()
	 * should immediately return NULL without blocking.
	 * Subsequent call to the initialize() callback should restore ff_stream_connector_connect()
	 * functionality.
	 */
	void (*shutdown)(struct ff_stream_connector *stream_connector);

	/**
	 * the connect() callback should return next connected stream or NULL on error
	 * or after the ff_stream_connector_shutdown() was called or if the ff_stream_connector_initialize()
	 * wasn't called before.
	 */
	struct ff_stream *(*connect)(struct ff_stream_connector *stream_connector);
};

/**
 * creates a stream connector using the given vtable and ctx.
 * ctx then can be obtained by ff_stream_connector_get_ctx() function.
 * vtable must be persistent until the ff_stream_connector_delete() will be called.
 * Always returns correct result.
 */
FF_API struct ff_stream_connector *ff_stream_connector_create(const struct ff_stream_connector_vtable *vtable, void *ctx);

/**
 * deletes the given stream_connector
 */
FF_API void ff_stream_connector_delete(struct ff_stream_connector *stream_connector);

/**
 * returns context, which was passed to the ff_stream_connector_create().
 */
FF_API void *ff_stream_connector_get_ctx(struct ff_stream_connector *stream_connector);

/**
 * Initializes the stream_connector.
 * This function should be called before calling into the ff_stream_connector_connect()
 * and after the ff_stream_connector_shutdown() was called.
 */
FF_API void ff_stream_connector_initialize(struct ff_stream_connector *stream_connector);

/**
 * Shutdowns the stream_connector. Currently blocked ff_stream_connector_connect() call
 * immediately unblocks and returns NULL. Subsequent calls to the ff_stream_connector_connect()
 * immediately return NULL without blocking.
 * The stream_connector can be restored by subsequent call into the ff_stream_connector_initialize(),
 * so the ff_stream_connector_connect() will work again.
 */
FF_API void ff_stream_connector_shutdown(struct ff_stream_connector *stream_connector);

/**
 * established connection and returs stream for this connection.
 * Returns NULL if the connection cannot be established.
 */
FF_API struct ff_stream *ff_stream_connector_connect(struct ff_stream_connector *stream_connector);

#ifdef __cplusplus
}
#endif

#endif
