#include "private/ff_common.h"

#include "private/ff_stream_tcp_with_timeout.h"

struct tcp_timeout_data
{
	struct ff_tcp *tcp;
	int read_timeout;
	int write_timeout;
};

static void delete_tcp(struct ff_stream *stream)
{
	struct tcp_timeout_data *data;

	data = (struct tcp_timeout_data *) ff_stream_get_ctx(stream);
	ff_tcp_delete(data->tcp);
	ff_free(data);
}

static int read_from_tcp(struct ff_stream *stream, void *buf, int len)
{
	struct tcp_timeout_data *data;
	int is_success;

	ff_assert(len >= 0);

	data = (struct tcp_timeout_data *) ff_stream_get_ctx(stream);
	ff_assert(data->read_timeout > 0);
	is_success = ff_tcp_read_with_timeout(data->tcp, buf, len, data->read_timeout);
	return is_success;
}

static int write_to_tcp(struct ff_stream *stream, const void *buf, int len)
{
	struct tcp_timeout_data *data;
	int is_success;

	ff_assert(len >= 0);

	data = (struct tcp_timeout_data *) ff_stream_get_ctx(stream);
	ff_assert(data->write_timeout > 0);
	is_success = ff_tcp_write_with_timeout(data->tcp, buf, len, data->write_timeout);
	return is_success;
}

static int flush_tcp(struct ff_stream *stream)
{
	struct tcp_timeout_data *data;
	int is_success;

	data = (struct tcp_timeout_data *) ff_stream_get_ctx(stream);
	is_success = ff_tcp_flush_with_timeout(data->tcp, data->write_timeout);
	return is_success;
}

static void disconnect_tcp(struct ff_stream *stream)
{
	struct tcp_timeout_data *data;

	data = (struct tcp_timeout_data *) ff_stream_get_ctx(stream);
	ff_tcp_disconnect(data->tcp);
}

static struct ff_stream_vtable tcp_stream_vtable =
{
	delete_tcp,
	read_from_tcp,
	write_to_tcp,
	flush_tcp,
	disconnect_tcp
};

struct ff_stream *ff_stream_tcp_with_timeout_create(struct ff_tcp *tcp, int read_timeout, int write_timeout)
{
	struct ff_stream *stream;
	struct tcp_timeout_data *data;

	ff_assert(read_timeout > 0);
	ff_assert(write_timeout > 0);

	data = (struct tcp_timeout_data *) ff_malloc(sizeof(*data));
	data->tcp = tcp;
	data->read_timeout = read_timeout;
	data->write_timeout = write_timeout;
	stream = ff_stream_create(&tcp_stream_vtable, data);
	return stream;
}
