#ifndef FF_ENDPOINT_PUBLIC
#define FF_ENDPOINT_PUBLIC

#include "ff/ff_common.h"
#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_endpoint;

struct ff_endpoint_vtable
{
	/**
	 * the delete() callback should release the context passed to the ff_endpoint_create()
	 */
	void (*delete)(struct ff_endpoint *endpoint);

	/**
	 * the accept() callback should return next accepted stream or NULL on error
	 * or after the ff_endpoint_disconnect() was called.
	 */
	struct ff_stream *(*accept)(struct ff_endpoint *endpoint);

	/**
	 * the disconnect() callback should unblock the currently blocked ff_endpoint_accept()
	 * functions. These functions should return NULL and subsequent calls to the ff_endpoint_accept()
	 * should immediately return NULL without blocking.
	 */
	void (*disconnect)(struct ff_endpoint *endpoint);
};

/**
 * Creates and endpoint using given vtable and ctx.
 * ctx then can be obtained by ff_endpoint_get_ctx() function.
 */
FF_API struct ff_endpoint *ff_endpoint_create(struct ff_endpoint_vtable *vtable, void *ctx);

/**
 * Returns context passed to the ff_endpoint_create().
 */
FF_API void *ff_endpoint_get_ctx(struct ff_endpoint *endpoint);

/**
 * Deletes the given endpoint.
 */
FF_API void ff_endpoint_delete(struct ff_endpoint *endpoint);

/**
 * Accepts the next stream from the endpoint.
 * Returns accepted stream on success or NULL on error
 * or after the ff_endpoint_disconnect() was called.
 */
FF_API struct ff_stream *ff_endpoint_accept(struct ff_endpoint *endpoint);

/**
 * Shutdowns the endpoint. Currently blocked ff_endpoint_accept() calls
 * immediately unblock and return NULL. Subsequent calls to the ff_endpoint_accept()
 * immediately return NULL without blocking.
 */
FF_API void ff_endpoint_disconnect(struct ff_endpoint *endpoint);

#ifdef __cplusplus
}
#endif

#endif
