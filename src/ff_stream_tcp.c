#include "private/ff_common.h"

#include "private/ff_stream_tcp.h"

static void delete_tcp(struct ff_stream *stream)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	ff_tcp_delete(tcp);
}

static enum ff_result read_from_tcp(struct ff_stream *stream, void *buf, int len)
{
	struct ff_tcp *tcp;
	enum ff_result result;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	result = ff_tcp_read(tcp, buf, len);
	return result;
}

static enum ff_result write_to_tcp(struct ff_stream *stream, const void *buf, int len)
{
	struct ff_tcp *tcp;
	enum ff_result result;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	result = ff_tcp_write(tcp, buf, len);
	return result;
}

static enum ff_result flush_tcp(struct ff_stream *stream)
{
	struct ff_tcp *tcp;
	enum ff_result result;

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	result = ff_tcp_flush(tcp);
	return result;
}

static void disconnect_tcp(struct ff_stream *stream)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	ff_tcp_disconnect(tcp);
}

static const struct ff_stream_vtable tcp_stream_vtable =
{
	delete_tcp,
	read_from_tcp,
	write_to_tcp,
	flush_tcp,
	disconnect_tcp
};

struct ff_stream *ff_stream_tcp_create(struct ff_tcp *tcp)
{
	struct ff_stream *stream;

	stream = ff_stream_create(&tcp_stream_vtable, tcp);
	return stream;
}
