#include "private/ff_common.h"

#include "private/ff_tcp.h"
#include "private/arch/ff_arch_tcp.h"
#include "private/ff_read_stream_buffer.h"
#include "private/ff_write_stream_buffer.h"
#include "private/ff_core.h"

static const int READ_BUFFER_SIZE = 0x10000;
static const int WRITE_BUFFER_SIZE = 0x10000;

struct ff_tcp
{
	struct ff_arch_tcp *tcp;
	struct ff_read_stream_buffer *read_buffer;
	struct ff_write_stream_buffer *write_buffer;
};

static int tcp_read_func(void *ctx, void *buf, int len)
{
	struct ff_tcp *tcp;
	int bytes_read;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ctx;
	bytes_read = ff_arch_tcp_read(tcp->tcp, buf, len);
	return bytes_read;
}

static int tcp_write_func(void *ctx, const void *buf, int len)
{
	struct ff_tcp *tcp;
	int bytes_written;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ctx;
	bytes_written = ff_arch_tcp_write(tcp->tcp, buf, len);
	return bytes_written;
}

static struct ff_tcp *create_from_arch_tcp(struct ff_arch_tcp *arch_tcp)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ff_malloc(sizeof(*tcp));
	tcp->tcp = arch_tcp;
	tcp->read_buffer = ff_read_stream_buffer_create(tcp_read_func, tcp, READ_BUFFER_SIZE);
	tcp->write_buffer = ff_write_stream_buffer_create(tcp_write_func, tcp, WRITE_BUFFER_SIZE);

	return tcp;
}

static void cancel_tcp_operation(struct ff_fiber *fiber, void *ctx)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ctx;
	ff_tcp_disconnect(tcp);
}

struct ff_tcp *ff_tcp_create()
{
	struct ff_tcp *tcp;
	struct ff_arch_tcp *arch_tcp;

	arch_tcp = ff_arch_tcp_create();
	tcp = create_from_arch_tcp(arch_tcp);

	return tcp;
}

void ff_tcp_delete(struct ff_tcp *tcp)
{
	ff_write_stream_buffer_delete(tcp->write_buffer);
	ff_read_stream_buffer_delete(tcp->read_buffer);
	ff_arch_tcp_delete(tcp->tcp);
	ff_free(tcp);
}

int ff_tcp_bind(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr, enum ff_tcp_type type)
{
	int is_success;
	int is_listening;

	is_listening = ((type == FF_TCP_SERVER) ? 1 : 0);
	is_success = ff_arch_tcp_bind(tcp->tcp, addr, is_listening);
	return is_success;
}

int ff_tcp_connect(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr)
{
	int is_success;

	is_success = ff_arch_tcp_connect(tcp->tcp, addr);
	return is_success;
}

struct ff_tcp *ff_tcp_accept(struct ff_tcp *tcp, struct ff_arch_net_addr *remote_addr)
{
	struct ff_arch_tcp *remote_arch_tcp;
	struct ff_tcp *remote_tcp = NULL;

	remote_arch_tcp = ff_arch_tcp_accept(tcp->tcp, remote_addr);
	if (remote_arch_tcp != NULL)
	{
		remote_tcp = create_from_arch_tcp(remote_arch_tcp);
	}
	return remote_tcp;
}

int ff_tcp_read(struct ff_tcp *tcp, void *buf, int len)
{
	int bytes_read;

	ff_assert(len >= 0);

	bytes_read = ff_read_stream_buffer_read(tcp->read_buffer, buf, len);
	return bytes_read;
}

int ff_tcp_read_with_timeout(struct ff_tcp *tcp, void *buf, int len, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	int bytes_read;

	ff_assert(len >= 0);
	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_tcp_operation, tcp);
	bytes_read = ff_tcp_read(tcp, buf, len);
	ff_core_deregister_timeout_operation(timeout_operation_data);

	return bytes_read;
}

int ff_tcp_write(struct ff_tcp *tcp, const void *buf, int len)
{
	int bytes_written;

	ff_assert(len >= 0);

	bytes_written = ff_write_stream_buffer_write(tcp->write_buffer, buf, len);
	return bytes_written;
}

int ff_tcp_write_with_timeout(struct ff_tcp *tcp, const void *buf, int len, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	int bytes_written;

	ff_assert(len >= 0);
	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_tcp_operation, tcp);
	bytes_written = ff_tcp_write(tcp, buf, len);
	ff_core_deregister_timeout_operation(timeout_operation_data);

	return bytes_written;
}

int ff_tcp_flush(struct ff_tcp *tcp)
{
	int bytes_written;

	bytes_written = ff_write_stream_buffer_flush(tcp->write_buffer);
	return bytes_written;
}

int ff_tcp_flush_with_timeout(struct ff_tcp *tcp, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	int bytes_written;

	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_tcp_operation, tcp);
	bytes_written = ff_tcp_flush(tcp);
	ff_core_deregister_timeout_operation(timeout_operation_data);

	return bytes_written;
}

void ff_tcp_disconnect(struct ff_tcp *tcp)
{
	ff_arch_tcp_disconnect(tcp->tcp);
}
