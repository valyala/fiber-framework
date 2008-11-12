#include "private/ff_common.h"

#include "private/ff_stream_connector_tcp.h"
#include "private/ff_stream_connector.h"
#include "private/arch/ff_arch_net_addr.h"
#include "private/ff_stream.h"
#include "private/ff_tcp.h"
#include "private/ff_stream_tcp.h"
#include "private/ff_event.h"

/**
 * timeout in milliseconds, which is used between unsuccessful reconnections
 */
#define RECONNECT_TIMEOUT 1000

struct tcp_stream_connector
{
	struct ff_arch_net_addr *addr;
	struct ff_event *must_shutdown_event;
	int is_initialized;
};

static void delete_tcp_stream_connector(struct ff_stream_connector *stream_connector)
{
	struct tcp_stream_connector *tcp_stream_connector;

	tcp_stream_connector = (struct tcp_stream_connector *) ff_stream_connector_get_ctx(stream_connector);
	ff_assert(!tcp_stream_connector->is_initialized);
	ff_event_delete(tcp_stream_connector->must_shutdown_event);
	ff_arch_net_addr_delete(tcp_stream_connector->addr);
	ff_free(tcp_stream_connector);
}

static void initialize_tcp_stream_connector(struct ff_stream_connector *stream_connector)
{
	struct tcp_stream_connector *tcp_stream_connector;

	tcp_stream_connector = (struct tcp_stream_connector *) ff_stream_connector_get_ctx(stream_connector);
	ff_assert(!tcp_stream_connector->is_initialized);
	tcp_stream_connector->is_initialized = 1;
	ff_event_reset(tcp_stream_connector->must_shutdown_event);
}

static void shutdown_tcp_stream_connector(struct ff_stream_connector *stream_connector)
{
	struct tcp_stream_connector *tcp_stream_connector;

	tcp_stream_connector = (struct tcp_stream_connector *) ff_stream_connector_get_ctx(stream_connector);
	if (tcp_stream_connector->is_initialized)
	{
		tcp_stream_connector->is_initialized = 0;
		ff_event_set(tcp_stream_connector->must_shutdown_event);
	}
	else
	{
		ff_log_debug(L"stream_connector=%p already has been shutdowned, so it won't be shutdowned again", stream_connector);
	}
}

static struct ff_stream *connect_tcp_stream_connector(struct ff_stream_connector *stream_connector)
{
	struct tcp_stream_connector *tcp_stream_connector;
	struct ff_stream *stream = NULL;
	
	tcp_stream_connector = (struct tcp_stream_connector *) ff_stream_connector_get_ctx(stream_connector);
	while (tcp_stream_connector->is_initialized)
	{
		struct ff_tcp *tcp;
		const wchar_t *str_addr;
		enum ff_result result;

		tcp = ff_tcp_create();
		result = ff_tcp_connect(tcp, tcp_stream_connector->addr);
		if (result == FF_SUCCESS)
		{
			stream = ff_stream_tcp_create(tcp);
			break;
		}
		ff_tcp_delete(tcp);

		str_addr = ff_arch_net_addr_to_string(tcp_stream_connector->addr);
		ff_log_debug(L"cannot establish connection to the address [%ls]", str_addr);
		ff_arch_net_addr_delete_string(str_addr);

		result = ff_event_wait_with_timeout(tcp_stream_connector->must_shutdown_event, RECONNECT_TIMEOUT);
		if (result == FF_SUCCESS)
		{
			/* must_shutdown_event can be set only in the shutdown_tcp_stream_connector() */
			ff_assert(!tcp_stream_connector->is_initialized);
			ff_log_debug(L"shutdown_tcp_stream_connector() has been called for the stream_connector=%p", stream_connector);
			break;
		}
	}

	return stream;
}

static const struct ff_stream_connector_vtable tcp_stream_connector_vtable =
{
	delete_tcp_stream_connector,
	initialize_tcp_stream_connector,
	shutdown_tcp_stream_connector,
	connect_tcp_stream_connector
};

struct ff_stream_connector *ff_stream_connector_tcp_create(struct ff_arch_net_addr *addr)
{
	struct tcp_stream_connector *tcp_stream_connector;
	struct ff_stream_connector *stream_connector;

	tcp_stream_connector = (struct tcp_stream_connector *) ff_malloc(sizeof(*tcp_stream_connector));
	tcp_stream_connector->addr = addr;
	tcp_stream_connector->must_shutdown_event = ff_event_create(FF_EVENT_MANUAL);
	tcp_stream_connector->is_initialized = 0;

	stream_connector = ff_stream_connector_create(&tcp_stream_connector_vtable, tcp_stream_connector);
	return stream_connector;
}
