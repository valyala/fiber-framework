#include "private/ff_common.h"

#include "private/ff_stream.h"

struct ff_stream
{
	struct ff_stream_vtable *vtable;
	void *ctx;
};

struct ff_stream *ff_stream_create(struct ff_stream_vtable *vtable, void *ctx)
{
	struct ff_stream *stream;

	stream = (struct ff_stream *) ff_malloc(sizeof(*stream));
	stream->vtable = vtable;
	stream->ctx = ctx;
	return stream;
}

void ff_stream_delete(struct ff_stream *stream)
{
	stream->vtable->delete(stream);
	ff_free(stream);
}

void *ff_stream_get_ctx(struct ff_stream *stream)
{
	return stream->ctx;
}

int ff_stream_read(struct ff_stream *stream, void *buf, int len)
{
	int is_success;

	ff_assert(len >= 0);

	is_success = stream->vtable->read(stream, buf, len);
	return is_success;
}

int ff_stream_write(struct ff_stream *stream, const void *buf, int len)
{
	int is_success;

	ff_assert(len >= 0);

	is_success = stream->vtable->write(stream, buf, len);
	return is_success;
}

int ff_stream_flush(struct ff_stream *stream)
{
	int is_success;

	is_success = stream->vtable->flush(stream);
	return is_success;
}

void ff_stream_disconnect(struct ff_stream *stream)
{
	stream->vtable->disconnect(stream);
}
