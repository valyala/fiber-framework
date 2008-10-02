#include "private/ff_common.h"

#include "private/ff_stream_tcp.h"

static void delete_tcp(struct ff_stream *stream)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	ff_tcp_delete(tcp);
}

static int read_from_tcp(struct ff_stream *stream, void *buf, int len)
{
	struct ff_tcp *tcp;
	int is_success;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	is_success = ff_tcp_read(tcp, buf, len);
	return is_success;
}

static int read_from_tcp_with_timeout(struct ff_stream *stream, void *buf, int len, int timeout)
{
	struct ff_tcp *tcp;
	int is_success;

	ff_assert(len >= 0);
	ff_assert(timeout > 0);

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	is_success = ff_tcp_read_with_timeout(tcp, buf, len, timeout);
	return is_success;
}

static int write_to_tcp(struct ff_stream *stream, const void *buf, int len)
{
	struct ff_tcp *tcp;
	int is_success;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	is_success = ff_tcp_write(tcp, buf, len);
	return is_success;
}

static int write_to_tcp_with_timeout(struct ff_stream *stream, const void *buf, int len, int timeout)
{
	struct ff_tcp *tcp;
	int is_success;

	ff_assert(len >= 0);
	ff_assert(timeout > 0);

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	is_success = ff_tcp_write_with_timeout(tcp, buf, len, timeout);
	return is_success;
}

static int flush_tcp(struct ff_stream *stream)
{
	struct ff_tcp *tcp;
	int is_success;

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	is_success = ff_tcp_flush(tcp);
	return is_success;
}

static void disconnect_tcp(struct ff_stream *stream)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ff_stream_get_ctx(stream);
	ff_tcp_disconnect(tcp);
}

static struct ff_stream_vtable tcp_stream_vtable =
{
	delete_tcp,
	read_from_tcp,
	read_from_tcp_with_timeout,
	write_to_tcp,
	write_to_tcp_with_timeout,
	flush_tcp,
	disconnect_tcp
};

struct ff_stream *ff_stream_tcp_create(struct ff_tcp *tcp)
{
	struct ff_stream *stream;

	stream = ff_stream_create(&tcp_stream_vtable, tcp);
	return stream;
}
