#include "private/ff_common.h"

#include "private/ff_endpoint_tcp.h"
#include "private/ff_endpoint.h"
#include "private/ff_tcp.h"
#include "private/ff_stream_tcp.h"
#include "private/ff_stream.h"
#include "private/arch/ff_arch_net_addr.h"

static void delete_tcp_endpoint(struct ff_endpoint *endpoint)
{
	struct ff_tcp *tcp_endpoint;

	tcp_endpoint = (struct ff_tcp *) ff_endpoint_get_ctx(endpoint);
	ff_tcp_delete(tcp_endpoint);
}

static struct ff_stream *accept_tcp_endpoint(struct ff_endpoint *endpoint)
{
	struct ff_tcp *tcp_endpoint;
	struct ff_tcp *tcp_client;
	struct ff_arch_net_addr *client_addr;
	struct ff_stream *client_stream = NULL;

	tcp_endpoint = (struct ff_tcp *) ff_endpoint_get_ctx(endpoint);
	client_addr = ff_arch_net_addr_create();
	tcp_client = ff_tcp_accept(tcp_endpoint, client_addr);
	ff_arch_net_addr_delete(client_addr);
	if (tcp_client != NULL)
	{
		client_stream = ff_stream_tcp_create(tcp_client);
	}

	return client_stream;
}

static void disconnect_tcp_endpoint(struct ff_endpoint *endpoint)
{
	struct ff_tcp *tcp_endpoint;

	tcp_endpoint = (struct ff_tcp *) ff_endpoint_get_ctx(endpoint);
	ff_tcp_disconnect(tcp_endpoint);
}

static const struct ff_endpoint_vtable tcp_endpoint_vtable =
{
	delete_tcp_endpoint,
	accept_tcp_endpoint,
	disconnect_tcp_endpoint
};

struct ff_endpoint *ff_endpoint_tcp_create(struct ff_tcp *tcp_endpoint)
{
	struct ff_endpoint *endpoint;

	endpoint = ff_endpoint_create(&tcp_endpoint_vtable, tcp_endpoint);
	return endpoint;
}
