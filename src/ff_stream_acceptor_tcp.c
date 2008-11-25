#include "private/ff_common.h"

#include "private/ff_stream_acceptor_tcp.h"
#include "private/ff_stream_acceptor.h"
#include "private/ff_tcp.h"
#include "private/ff_stream_tcp.h"
#include "private/ff_stream.h"
#include "private/arch/ff_arch_net_addr.h"

struct tcp_stream_acceptor
{
	struct ff_tcp *tcp;
	struct ff_arch_net_addr *addr;
	int is_initialized;
};

static void delete_tcp_stream_acceptor(void *ctx)
{
	struct tcp_stream_acceptor *tcp_stream_acceptor;

	tcp_stream_acceptor = (struct tcp_stream_acceptor *) ctx;
	ff_assert(!tcp_stream_acceptor->is_initialized);
	ff_tcp_delete(tcp_stream_acceptor->tcp);
	ff_arch_net_addr_delete(tcp_stream_acceptor->addr);
	ff_free(tcp_stream_acceptor);
}

static void initialize_tcp_stream_acceptor(void *ctx)
{
	struct tcp_stream_acceptor *tcp_stream_acceptor;
	enum ff_result result;

	tcp_stream_acceptor = (struct tcp_stream_acceptor *) ctx;
	ff_assert(!tcp_stream_acceptor->is_initialized);
	ff_tcp_delete(tcp_stream_acceptor->tcp);
	tcp_stream_acceptor->tcp = ff_tcp_create();
	result = ff_tcp_bind(tcp_stream_acceptor->tcp, tcp_stream_acceptor->addr, FF_TCP_SERVER);
	if (result != FF_SUCCESS)
	{
		const wchar_t *str_addr;

		str_addr = ff_arch_net_addr_to_string(tcp_stream_acceptor->addr);
		ff_log_fatal_error(L"cannot bind the given tcp stream_acceptor address %ls", str_addr);
		/* there is no need to call ff_arch_net_addr_delete_string(str_addr) here,
		 * because ff_log_fatal_error() will terminate the current program
		 */
	}
	else
	{
		tcp_stream_acceptor->is_initialized = 1;
	}
}

static void shutdown_tcp_stream_acceptor(void *ctx)
{
	struct tcp_stream_acceptor *tcp_stream_acceptor;

	tcp_stream_acceptor = (struct tcp_stream_acceptor *) ctx;
	if (tcp_stream_acceptor->is_initialized)
	{
		tcp_stream_acceptor->is_initialized = 0;
		ff_tcp_disconnect(tcp_stream_acceptor->tcp);
	}
	else
	{
		ff_log_debug(L"tcp_stream_acceptor=%p has been already shutdowned, so it won't be shutdowned again", tcp_stream_acceptor);
	}
}

static struct ff_stream *accept_tcp_stream_acceptor(void *ctx)
{
	struct tcp_stream_acceptor *tcp_stream_acceptor;
	struct ff_stream *client_stream = NULL;

	tcp_stream_acceptor = (struct tcp_stream_acceptor *) ctx;
	if (tcp_stream_acceptor->is_initialized)
	{
		struct ff_tcp *tcp_client;
		struct ff_arch_net_addr *client_addr;

		client_addr = ff_arch_net_addr_create();
		tcp_client = ff_tcp_accept(tcp_stream_acceptor->tcp, client_addr);
		ff_arch_net_addr_delete(client_addr);
		if (tcp_client != NULL)
		{
			client_stream = ff_stream_tcp_create(tcp_client);
		}
		else
		{
			/* ff_tcp_accept() can return NULL only if shutdown_tcp_stream_acceptor() was called */
			ff_assert(!tcp_stream_acceptor->is_initialized);
			ff_log_debug(L"shutdown_tcp_stream_acceptor() has been called for the tcp_stream_acceptor=%p. See previous messages for more info", tcp_stream_acceptor);
		}
    }
    else
    {
    	ff_log_debug(L"tcp_stream_acceptor=%p has been already shutdowned, so it can't be used for accepting connections", tcp_stream_acceptor);
    }

	return client_stream;
}

static const struct ff_stream_acceptor_vtable tcp_stream_acceptor_vtable =
{
	delete_tcp_stream_acceptor,
	initialize_tcp_stream_acceptor,
	shutdown_tcp_stream_acceptor,
	accept_tcp_stream_acceptor
};

struct ff_stream_acceptor *ff_stream_acceptor_tcp_create(struct ff_arch_net_addr *addr)
{
	struct ff_stream_acceptor *stream_acceptor;
	struct tcp_stream_acceptor *tcp_stream_acceptor;

	tcp_stream_acceptor = (struct tcp_stream_acceptor *) ff_malloc(sizeof(*tcp_stream_acceptor));
	tcp_stream_acceptor->tcp = ff_tcp_create();
	tcp_stream_acceptor->addr = addr;
	tcp_stream_acceptor->is_initialized = 0;

	stream_acceptor = ff_stream_acceptor_create(&tcp_stream_acceptor_vtable, tcp_stream_acceptor);
	return stream_acceptor;
}
