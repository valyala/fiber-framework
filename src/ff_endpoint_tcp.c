#include "private/ff_common.h"

#include "private/ff_endpoint_tcp.h"
#include "private/ff_endpoint.h"
#include "private/ff_tcp.h"
#include "private/ff_stream_tcp.h"
#include "private/ff_stream.h"
#include "private/arch/ff_arch_net_addr.h"
#include "private/ff_log.h"

struct tcp_endpoint
{
	struct ff_tcp *tcp;
	struct ff_arch_net_addr *addr;
};

static void delete_tcp_endpoint(struct ff_endpoint *endpoint)
{
	struct tcp_endpoint *tcp_endpoint;

	tcp_endpoint = (struct tcp_endpoint *) ff_endpoint_get_ctx(endpoint);
	ff_tcp_delete(tcp_endpoint->tcp);
	ff_arch_net_addr_delete(tcp_endpoint->addr);
	ff_free(tcp_endpoint);
}

static void initialize_tcp_endpoint(struct ff_endpoint *endpoint)
{
	struct tcp_endpoint *tcp_endpoint;
	enum ff_result result;

	tcp_endpoint = (struct tcp_endpoint *) ff_endpoint_get_ctx(endpoint);
	ff_tcp_delete(tcp_endpoint->tcp);
	tcp_endpoint->tcp = ff_tcp_create();
	result = ff_tcp_bind(tcp_endpoint->tcp, tcp_endpoint->addr, FF_TCP_SERVER);
	if (result != FF_SUCCESS)
	{
		const wchar_t *str_addr;
		
		str_addr = ff_arch_net_addr_to_string(tcp_endpoint->addr);
		ff_log_fatal_error(L"cannot bind the given tcp endpoint address %ls", str_addr);
		/* there is no need to call ff_arch_net_addr_delete_string(str_addr) here,
		 * because ff_log_fatal_error() will terminate the current program
		 */
	}
}

static void shutdown_tcp_endpoint(struct ff_endpoint *endpoint)
{
	struct tcp_endpoint *tcp_endpoint;

	tcp_endpoint = (struct tcp_endpoint *) ff_endpoint_get_ctx(endpoint);
	ff_tcp_disconnect(tcp_endpoint->tcp);
}

static struct ff_stream *accept_tcp_endpoint(struct ff_endpoint *endpoint)
{
	struct tcp_endpoint *tcp_endpoint;
	struct ff_tcp *tcp_client;
	struct ff_arch_net_addr *client_addr;
	struct ff_stream *client_stream = NULL;

	tcp_endpoint = (struct tcp_endpoint *) ff_endpoint_get_ctx(endpoint);
	client_addr = ff_arch_net_addr_create();
	tcp_client = ff_tcp_accept(tcp_endpoint->tcp, client_addr);
	ff_arch_net_addr_delete(client_addr);
	if (tcp_client != NULL)
	{
		client_stream = ff_stream_tcp_create(tcp_client);
	}

	return client_stream;
}

static const struct ff_endpoint_vtable tcp_endpoint_vtable =
{
	delete_tcp_endpoint,
	initialize_tcp_endpoint,
	shutdown_tcp_endpoint,
	accept_tcp_endpoint
};

struct ff_endpoint *ff_endpoint_tcp_create(struct ff_arch_net_addr *addr)
{
	struct ff_endpoint *endpoint;
	struct tcp_endpoint *tcp_endpoint;

	tcp_endpoint = (struct tcp_endpoint *) ff_malloc(sizeof(*tcp_endpoint));
	tcp_endpoint->tcp = ff_tcp_create();
	tcp_endpoint->addr = addr;

	endpoint = ff_endpoint_create(&tcp_endpoint_vtable, tcp_endpoint);
	return endpoint;
}
