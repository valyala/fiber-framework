#include "private/ff_common.h"

#include "private/ff_tcp.h"
#include "private/arch/ff_arch_tcp.h"
#include "private/ff_read_stream_buffer.h"
#include "private/ff_write_stream_buffer.h"
#include "private/ff_core.h"

#define READ_BUFFER_SIZE 0x10000
#define WRITE_BUFFER_SIZE 0x10000

struct ff_tcp
{
	struct ff_arch_tcp *tcp;
	struct ff_read_stream_buffer *read_buffer;
	struct ff_write_stream_buffer *write_buffer;
	int is_active;
};

static int tcp_read_func(void *ctx, void *buf, int len)
{
	struct ff_tcp *tcp;
	int bytes_read;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ctx;
	bytes_read = ff_arch_tcp_read(tcp->tcp, buf, len);
	if (bytes_read == -1)
	{
		ff_log_debug(L"error while reading from the tcp=%p to the buf=%p, len=%d. See previous messages for more info", tcp, buf, len);
	}
	return bytes_read;
}

static int tcp_write_func(void *ctx, const void *buf, int len)
{
	struct ff_tcp *tcp;
	int bytes_written;

	ff_assert(len >= 0);

	tcp = (struct ff_tcp *) ctx;
	bytes_written = ff_arch_tcp_write(tcp->tcp, buf, len);
	if (bytes_written == -1)
	{
		ff_log_debug(L"error while writing to the tcp=%p from the buf=%p, len=%d. See previous messages for more info", tcp, buf, len);
	}
	return bytes_written;
}

static struct ff_tcp *create_from_arch_tcp(struct ff_arch_tcp *arch_tcp)
{
	struct ff_tcp *tcp;

	tcp = (struct ff_tcp *) ff_malloc(sizeof(*tcp));
	tcp->tcp = arch_tcp;
	tcp->read_buffer = ff_read_stream_buffer_create(tcp_read_func, tcp, READ_BUFFER_SIZE);
	tcp->write_buffer = ff_write_stream_buffer_create(tcp_write_func, tcp, WRITE_BUFFER_SIZE);
	tcp->is_active = 0;

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

enum ff_result ff_tcp_bind(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr, enum ff_tcp_type type)
{
	int is_listening;
	enum ff_result result;

	ff_assert(!tcp->is_active);

	is_listening = ((type == FF_TCP_SERVER) ? 1 : 0);
	result = ff_arch_tcp_bind(tcp->tcp, addr, is_listening);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot bind the tcp=%p to the addr=%p, is_listening=%d. See previous messages for more info", tcp, addr, is_listening);
	}
	if (result == FF_SUCCESS && is_listening)
	{
		tcp->is_active = 1;
	}
	return result;
}

enum ff_result ff_tcp_connect(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr)
{
	enum ff_result result;

	ff_assert(!tcp->is_active);

	result = ff_arch_tcp_connect(tcp->tcp, addr);
	if (result == FF_SUCCESS)
	{
		tcp->is_active = 1;
	}
	else
	{
		ff_log_debug(L"cannot connect tcp=%p to the addr=%p. See previous messages for more info", tcp, addr);
	}
	return result;
}

struct ff_tcp *ff_tcp_accept(struct ff_tcp *tcp, struct ff_arch_net_addr *remote_addr)
{
	struct ff_arch_tcp *remote_arch_tcp;
	struct ff_tcp *remote_tcp = NULL;

	if (tcp->is_active)
	{
		remote_arch_tcp = ff_arch_tcp_accept(tcp->tcp, remote_addr);
		if (remote_arch_tcp != NULL)
		{
			remote_tcp = create_from_arch_tcp(remote_arch_tcp);
			remote_tcp->is_active = 1;
		}
		else
		{
			ff_log_debug(L"error while accepting connection on the tcp=%p, remote_addr=%p. See previous messages for more info", tcp, remote_addr);
		}
	}
	else
	{
		ff_log_debug(L"the tcp=%p was already disconnected, so it cannot be used for accepting new connections", tcp);
	}
	return remote_tcp;
}

enum ff_result ff_tcp_read(struct ff_tcp *tcp, void *buf, int len)
{
	enum ff_result result = FF_FAILURE;

	ff_assert(len >= 0);

	if (tcp->is_active)
	{
		result = ff_read_stream_buffer_read(tcp->read_buffer, buf, len);
		if (result != FF_SUCCESS)
		{
			ff_log_debug(L"error while reading data from the read_buffer=%p to buf=%p, len=%d. See previous messages for more info", tcp->read_buffer, buf, len);
		}
	}
	else
	{
		ff_log_debug(L"the tcp=%p was already disconnected, so it cannot be used for reading data to the buf=%p, len=%d", tcp, buf, len);
	}
	return result;
}

enum ff_result ff_tcp_read_with_timeout(struct ff_tcp *tcp, void *buf, int len, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	enum ff_result result;
	enum ff_result tmp_result;

	ff_assert(len >= 0);
	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_tcp_operation, tcp);
	result = ff_tcp_read(tcp, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while reading data from the tcp=%p to the buf=%p, len=%d using timeout=%d. See previous messages for more info", tcp, buf, len, timeout);
	}
	tmp_result = ff_core_deregister_timeout_operation(timeout_operation_data);
	if (tmp_result != FF_SUCCESS)
	{
		ff_log_debug(L"timeout=%d has been exceeded for read operation from the tcp=%p to the buf=%p, len=%d", timeout, tcp, buf, len);
	}

	return result;
}

enum ff_result ff_tcp_write(struct ff_tcp *tcp, const void *buf, int len)
{
	enum ff_result result = FF_FAILURE;

	ff_assert(len >= 0);

	if (tcp->is_active)
	{
		result = ff_write_stream_buffer_write(tcp->write_buffer, buf, len);
		if (result != FF_SUCCESS)
		{
			ff_log_debug(L"error while writing data to the write_buffer=%p from the buf=%p, len=%d. See previous messages for more info", tcp->write_buffer, buf, len);
		}
	}
	else
	{
		ff_log_debug(L"the tcp=%p was already disconnected, so it cannot be used for writing data from the buf=%p, len=%d", tcp, buf, len);
	}
	return result;
}

enum ff_result ff_tcp_write_with_timeout(struct ff_tcp *tcp, const void *buf, int len, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	enum ff_result result;
	enum ff_result tmp_result;

	ff_assert(len >= 0);
	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_tcp_operation, tcp);
	result = ff_tcp_write(tcp, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while writing data to the tcp=%p from the buf=%p, len=%d using timeout=%d. See previous messages for more info", tcp, buf, len, timeout);
	}
	tmp_result = ff_core_deregister_timeout_operation(timeout_operation_data);
	if (tmp_result != FF_SUCCESS)
	{
		ff_log_debug(L"timeout=%d has been exceeded for write operation to the tcp=%p from the buf=%p, len=%d", timeout, tcp, buf, len);
	}

	return result;
}

enum ff_result ff_tcp_flush(struct ff_tcp *tcp)
{
	enum ff_result result = FF_FAILURE;

	if (tcp->is_active)
	{
		result = ff_write_stream_buffer_flush(tcp->write_buffer);
		if (result != FF_SUCCESS)
		{
			ff_log_debug(L"error while flushing the write_buffer=%p. See previous messages for more info", tcp->write_buffer);
		}
	}
	else
	{
		ff_log_debug(L"the tcp=%p was already disconnected, so it cannot be flushed", tcp);
	}
	return result;
}

enum ff_result ff_tcp_flush_with_timeout(struct ff_tcp *tcp, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	enum ff_result result;
	enum ff_result tmp_result;

	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_tcp_operation, tcp);
	result = ff_tcp_flush(tcp);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while flushing the tcp=%p using timeout=%d. See previous messages for more info", tcp, timeout);
	}
	tmp_result = ff_core_deregister_timeout_operation(timeout_operation_data);
	if (tmp_result != FF_SUCCESS)
	{
		ff_log_debug(L"timeout=%d has been exceeded for flush operation on the tcp=%p", timeout, tcp);
	}

	return result;
}

void ff_tcp_disconnect(struct ff_tcp *tcp)
{
	if (tcp->is_active)
	{
		tcp->is_active = 0;
		ff_arch_tcp_disconnect(tcp->tcp);
	}
	else
	{
		ff_log_debug(L"the tcp=%p was already disconnected, so it won't be disconnected again", tcp);
	}
}
