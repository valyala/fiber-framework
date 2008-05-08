#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_tcp.h"
#include "private/ff_core.h"
#include "private/arch/ff_arch_completion_port.h"
#include "ff_win_completion_port.h"
#include "ff_win_error_check.h"

#include <winsock2.h>
#include <Mswsock.h>

struct ff_arch_tcp
{
	SOCKET handle;
	int is_disconnected;
};

struct ff_arch_tcp_addr
{
	struct sockaddr_in addr;
};

struct tcp_data
{
	LPFN_CONNECTEX connect_ex;
	struct ff_arch_completion_port *completion_port;
	SOCKET aux_socket;
};

static struct tcp_data tcp_ctx;

static void cancel_io_operation(struct ff_fiber *fiber, void *ctx)
{
	struct ff_arch_tcp *tcp;

	tcp = (struct ff_arch_tcp *) ctx;
	ff_arch_tcp_disconnect(tcp);
}

static int complete_overlapped_io(struct ff_arch_tcp *tcp, WSAOVERLAPPED *overlapped, int timeout)
{
	struct ff_fiber *current_fiber;
	int int_bytes_transferred = -1;
	int is_success;

	current_fiber = ff_core_get_current_fiber();
	ff_win_completion_port_register_overlapped_data(tcp_ctx.completion_port, overlapped, current_fiber);
	is_success = ff_core_do_timeout_operation(timeout, cancel_io_operation, tcp);
	ff_win_completion_port_deregister_overlapped_data(tcp_ctx.completion_port, overlapped);
	if (is_success)
	{
		BOOL result;
		DWORD flags;
		DWORD bytes_transferred;

		result = WSAGetOverlappedResult(tcp->handle, overlapped, &bytes_transferred, FALSE, &flags);
		if (result != FALSE)
		{
			int_bytes_transferred = (int) bytes_transferred;
		}
	}

	return int_bytes_transferred;
}

void ff_win_tcp_initialize(struct ff_arch_completion_port *completion_port)
{
	int rv;
	WORD version;
	struct WSAData wsa;
	GUID connect_ex_guid = WSAID_CONNECTEX;
	DWORD len;

	tcp_ctx.completion_port = completion_port;

	version = MAKEWORD(2, 2);
	rv = WSAStartup(version, &wsa);
	ff_winsock_fatal_error_check(rv == 0, L"cannot initialize winsock");

	tcp_ctx.aux_socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ff_winsock_fatal_error_check(tcp.aux_socket != INVALID_SOCKET, L"cannot create auxiliary socket");

	tcp_ctx.connect_ex = NULL;
	rv = WSAIoctl(tcp_ctx.aux_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &connect_ex_guid, sizeof(connect_ex_guid),
		&tcp_ctx.connect_ex, sizeof(tcp_ctx.connect_ex), &len, NULL, NULL);
	ff_winsock_fatal_error_check(rv == 0, L"cannot obtain ConnectEx() function");
	ff_assert(tcp_ctx.connect_ex != NULL);
}

void ff_win_tcp_shutdown()
{
	int rv;

	rv = closesocket(tcp_ctx.aux_socket);
	ff_assert(rv == 0);

	rv = WSACleanup();
	ff_assert(rv == 0);
}

struct ff_arch_tcp *ff_arch_tcp_create()
{
	HANDLE handle;
	struct ff_arch_tcp *tcp;

	tcp = (struct ff_arch_tcp *) ff_malloc(sizeof(*tcp));
	tcp->handle = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ff_winsock_fatal_error_check(tcp->handle != INVALID_SOCKET, L"cannot create tcp socket");
	tcp->is_disconnected = 0;

	handle = (HANDLE) tcp->handle;
	ff_win_completion_port_register_handle(tcp_ctx.completion_port, handle);

	return tcp;
}

void ff_arch_tcp_delete(struct ff_arch_tcp *tcp)
{
	int rv;

	rv = closesocket(tcp->handle);
	ff_assert(rv == 0);
	ff_free(tcp);
}

int ff_arch_tcp_connect(struct ff_arch_tcp *tcp, const struct ff_arch_tcp_addr *addr)
{
	int is_connected = 0;
	BOOL result;
	WSAOVERLAPPED overlapped;
	int bytes_transferred;

	memset(&overlapped, 0, sizeof(overlapped));
	result = tcp_ctx.connect_ex(tcp->handle, (struct sockaddr *) &addr->addr, sizeof(addr->addr), NULL, 0, NULL, &overlapped);
	if (result == FALSE)
	{
		int last_error;

		last_error = WSAGetLastError();
		if (last_error != WSA_IO_PENDING)
		{
			goto end;
		}
	}

	bytes_transferred = complete_overlapped_io(tcp, &overlapped, 0);
	if (bytes_transferred == -1)
	{
		goto end;
	}

	is_connected = 1;

end:
	return is_connected;
}

int ff_arch_tcp_read(struct ff_arch_tcp *tcp, void *buf, int len)
{
	int bytes_read;

	bytes_read = ff_arch_tcp_read_with_timeout(tcp, buf, len, 0);
	return bytes_read;
}

int ff_arch_tcp_read_with_timeout(struct ff_arch_tcp *tcp, void *buf, int len, int timeout)
{
	int rv;
	WSAOVERLAPPED overlapped;
	WSABUF wsa_buf;
	DWORD bytes_read;
	int int_bytes_read = -1;
	DWORD flags = 0;

	if (tcp->is_disconnected)
	{
		goto end;
	}

	wsa_buf.len = 1;
	wsa_buf.buf = (char *) buf;
	memset(&overlapped, 0, sizeof(overlapped));
	rv = WSARecv(tcp->handle, &wsa_buf, 1, &bytes_read, &flags, &overlapped, NULL);
	if (rv != 0)
	{
		int last_error;
		
		last_error = WSAGetLastError();
		if (last_error != WSA_IO_PENDING)
		{
			goto end;
		}
	}

	int_bytes_read = complete_overlapped_io(tcp, &overlapped, timeout);

end:
	return int_bytes_read;
}

int ff_arch_tcp_write(struct ff_arch_tcp *tcp, const void *buf, int len)
{
	int bytes_written;

	bytes_written = ff_arch_tcp_write_with_timeout(tcp, buf, len, 0);
	return bytes_written;
}

int ff_arch_tcp_write_with_timeout(struct ff_arch_tcp *tcp, const void *buf, int len, int timeout)
{
	int rv;
	WSAOVERLAPPED overlapped;
	WSABUF wsa_buf;
	DWORD bytes_written;
	int int_bytes_written = -1;
	DWORD flags = 0;

	if (tcp->is_disconnected)
	{
		goto end;
	}

	wsa_buf.len = 1;
	wsa_buf.buf = (char *) buf;
	memset(&overlapped, 0, sizeof(overlapped));
	rv = WSASend(tcp->handle, &wsa_buf, 1, &bytes_written, flags, &overlapped, NULL);
	if (rv != 0)
	{
		int last_error;
		
		last_error = WSAGetLastError();
		if (last_error != WSA_IO_PENDING)
		{
			goto end;
		}
	}

	int_bytes_written = complete_overlapped_io(tcp, &overlapped, timeout);

end:
	return int_bytes_written;
}

void ff_arch_tcp_disconnect(struct ff_arch_tcp *tcp)
{
	if (!tcp->is_disconnected)
	{
		BOOL result;

		result = CancelIo((HANDLE) tcp->handle);
		ff_assert(result != FALSE);
		tcp->is_disconnected = 1;
	}
}
