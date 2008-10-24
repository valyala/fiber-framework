#include "private/ff_common.h"

#include "private/ff_stream_connector_tcp.h"
#include "private/ff_stream_connector.h"
#include "private/arch/ff_arch_net_addr.h"
#include "private/ff_stream.h"
#include "private/ff_tcp.h"
#include "private/ff_stream_tcp.h"

static void delete_tcp_stream_connector(struct ff_stream_connector *stream_connector)
{
	struct ff_arch_net_addr *addr;

	addr = (struct ff_arch_net_addr *) ff_stream_connector_get_ctx(stream_connector);
	ff_arch_net_addr_delete(addr);
}

static struct ff_stream *connect_tcp_stream_connector(struct ff_stream_connector *stream_connector)
{
	struct ff_stream *stream = NULL;
	struct ff_arch_net_addr *addr;
	struct ff_tcp *tcp;
	enum ff_result result;

	addr = (struct ff_arch_net_addr *) ff_stream_connector_get_ctx(stream_connector);
	tcp = ff_tcp_create();
	result = ff_tcp_connect(tcp, addr);
	if (result == FF_SUCCESS)
	{
		stream = ff_stream_tcp_create(tcp);
	}
	else
	{
		ff_tcp_delete(tcp);
	}
	return stream;
}

static const struct ff_stream_connector_vtable tcp_stream_connector_vtable =
{
	delete_tcp_stream_connector,
	connect_tcp_stream_connector
};

struct ff_stream_connector *ff_stream_connector_tcp_create(struct ff_arch_net_addr *addr)
{
	struct ff_stream_connector *stream_connector;

	stream_connector = ff_stream_connector_create(&tcp_stream_connector_vtable, addr);
	return stream_connector;
}
