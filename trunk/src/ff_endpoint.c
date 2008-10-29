#include "private/ff_common.h"

#include "private/ff_endpoint.h"
#include "private/ff_stream.h"

struct ff_endpoint
{
	const struct ff_endpoint_vtable *vtable;
	void *ctx;
};

struct ff_endpoint *ff_endpoint_create(const struct ff_endpoint_vtable *vtable, void *ctx)
{
	struct ff_endpoint *endpoint;

	endpoint = (struct ff_endpoint *) ff_malloc(sizeof(*endpoint));
	endpoint->vtable = vtable;
	endpoint->ctx = ctx;

	return endpoint;
}

void ff_endpoint_delete(struct ff_endpoint *endpoint)
{
	endpoint->vtable->delete(endpoint);
	ff_free(endpoint);
}

void *ff_endpoint_get_ctx(struct ff_endpoint *endpoint)
{
	return endpoint->ctx;
}

void ff_endpoint_initialize(struct ff_endpoint *endpoint)
{
	endpoint->vtable->initialize(endpoint);
}

void ff_endpoint_shutdown(struct ff_endpoint *endpoint)
{
	endpoint->vtable->shutdown(endpoint);
}

struct ff_stream *ff_endpoint_accept(struct ff_endpoint *endpoint)
{
	struct ff_stream *stream;

	stream = endpoint->vtable->accept(endpoint);
	return stream;
}
