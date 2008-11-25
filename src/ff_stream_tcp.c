#include "private/ff_common.h"

#include "private/ff_stream_tcp.h"

static void delete_tcp(void *ctx)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ctx;
	ff_tcp_delete(tcp);
}

static enum ff_result read_from_tcp(void *ctx, void *buf, int len)
{
	struct ff_tcp *tcp;
	enum ff_result result;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ctx;
	result = ff_tcp_read(tcp, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while reading from the tcp=%p to the buf=%p, len=%d. See previous messages for more info", tcp, buf, len);
	}
	return result;
}

static enum ff_result write_to_tcp(void *ctx, const void *buf, int len)
{
	struct ff_tcp *tcp;
	enum ff_result result;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ctx;
	result = ff_tcp_write(tcp, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while writing to the tcp=%p from the buf=%p, len=%d. See previous messages for more info", tcp, buf, len);
	}
	return result;
}

static enum ff_result flush_tcp(void *ctx)
{
	struct ff_tcp *tcp;
	enum ff_result result;

	tcp = (struct ff_tcp *) ctx;
	result = ff_tcp_flush(tcp);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while flushing the tcp=%p. See previous messages for more info", tcp);
	}
	return result;
}

static void disconnect_tcp(void *ctx)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ctx;
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
