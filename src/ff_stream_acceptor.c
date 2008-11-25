#include "private/ff_common.h"

#include "private/ff_stream_acceptor.h"
#include "private/ff_stream.h"

struct ff_stream_acceptor
{
	const struct ff_stream_acceptor_vtable *vtable;
	void *ctx;
};

struct ff_stream_acceptor *ff_stream_acceptor_create(const struct ff_stream_acceptor_vtable *vtable, void *ctx)
{
	struct ff_stream_acceptor *stream_acceptor;

	stream_acceptor = (struct ff_stream_acceptor *) ff_malloc(sizeof(*stream_acceptor));
	stream_acceptor->vtable = vtable;
	stream_acceptor->ctx = ctx;

	return stream_acceptor;
}

void ff_stream_acceptor_delete(struct ff_stream_acceptor *stream_acceptor)
{
	stream_acceptor->vtable->delete(stream_acceptor->ctx);
	ff_free(stream_acceptor);
}

void ff_stream_acceptor_initialize(struct ff_stream_acceptor *stream_acceptor)
{
	stream_acceptor->vtable->initialize(stream_acceptor->ctx);
}

void ff_stream_acceptor_shutdown(struct ff_stream_acceptor *stream_acceptor)
{
	stream_acceptor->vtable->shutdown(stream_acceptor->ctx);
}

struct ff_stream *ff_stream_acceptor_accept(struct ff_stream_acceptor *stream_acceptor)
{
	struct ff_stream *stream;

	stream = stream_acceptor->vtable->accept(stream_acceptor->ctx);
	if (stream == NULL)
	{
		ff_log_debug(L"cannot accept connection using the stream_acceptor=%p. See previous messages for more info", stream_acceptor);
	}
	return stream;
}
