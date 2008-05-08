#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_tcp.h"
#include "private/ff_core.h"
#include "private/arch/ff_arch_completion_port.h"
#include "ff_win_completion_port.h"
#include "ff_win_error_check.h"

#include <winsock2.h>
#include <Mswsock.h>
#include <ws2tcpip.h>

#define MAX_STR_PORT_SIZE 6

struct ff_arch_tcp
{
	SOCKET handle;
	int is_disconnected;
};

struct ff_arch_tcp_addr
{
	struct sockaddr_in addr;
};

struct threadpool_addr_resolve_data
{
	struct ff_arch_tcp_addr *addr;
	const wchar_t *host;
	int port;
	int is_success;
};

struct tcp_data
{
	struct ff_arch_completion_port *completion_port;
	SOCKET aux_socket;
	LPFN_CONNECTEX connect_ex;
	LPFN_ACCEPTEX accept_ex;
	LPFN_GETACCEPTEXSOCKADDRS get_accept_ex_sockaddrs;
};

static struct tcp_data tcp_ctx;

static int resolve_addr(struct ff_arch_tcp_addr *addr, const wchar_t *host, int port)
{
	int is_success = 0;
	int rv;
	ADDRINFOW hint;
	PADDRINFOW addr_info_ptr;
	wchar_t str_port[MAX_STR_PORT_SIZE];

	ff_assert(port >= 0);
	ff_assert(port < 0x10000);

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;

	rv = swprintf_s(str_port, MAX_STR_PORT_SIZE, L"%d", port);
	ff_assert(rv != -1);
	addr_info_ptr = NULL;
	rv = GetAddrInfoW(host, str_port, &hint, &addr_info_ptr);
	if (rv != 0)
	{
		goto end;
	}
	ff_assert(addr_info_ptr != NULL);

	ff_assert(addr_info_ptr->ai_addrlen == sizeof(addr->addr));
	memcpy(&addr->addr, addr_info_ptr->ai_addr, sizeof(addr->addr));
	FreeAddrInfoW(addr_info_ptr);
	is_success = 1;

end:
	return is_success;
}

static void threadpool_addr_resolve_func(void *ctx)
{
	struct threadpool_addr_resolve_data *data;

	data = (struct threadpool_addr_resolve_data *) ctx;
	data->is_success = resolve_addr(data->addr, data->host, data->port);
}

static SOCKET create_tcp_socket()
{
	SOCKET socket;

	socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ff_winsock_fatal_error_check(socket != INVALID_SOCKET, L"cannot create tcp socket");

	return socket;
}

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
	DWORD len;
	GUID connect_ex_guid = WSAID_CONNECTEX;
	GUID accept_ex_guid = WSAID_ACCEPTEX;
	GUID get_accept_ex_sockaddrs_guid = WSAID_GETACCEPTEXSOCKADDRS;

	tcp_ctx.completion_port = completion_port;

	version = MAKEWORD(2, 2);
	rv = WSAStartup(version, &wsa);
	ff_winsock_fatal_error_check(rv == 0, L"cannot initialize winsock");

	tcp_ctx.aux_socket = create_tcp_socket();

	tcp_ctx.connect_ex = NULL;
	rv = WSAIoctl(tcp_ctx.aux_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &connect_ex_guid, sizeof(connect_ex_guid),
		&tcp_ctx.connect_ex, sizeof(tcp_ctx.connect_ex), &len, NULL, NULL);
	ff_winsock_fatal_error_check(rv == 0, L"cannot obtain ConnectEx() function");
	ff_assert(tcp_ctx.connect_ex != NULL);

	tcp_ctx.accept_ex = NULL;
	rv = WSAIoctl(tcp_ctx.aux_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &accept_ex_guid, sizeof(accept_ex_guid),
		&tcp_ctx.accept_ex, sizeof(tcp_ctx.accept_ex), &len, NULL, NULL);
	ff_winsock_fatal_error_check(rv == 0, L"cannot obtain AcceptEx() function");
	ff_assert(tcp_ctx.accept_ex != NULL);

	tcp_ctx.get_accept_ex_sockaddrs = NULL;
	rv = WSAIoctl(tcp_ctx.aux_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &get_accept_ex_sockaddrs_guid, sizeof(get_accept_ex_sockaddrs_guid),
		&tcp_ctx.get_accept_ex_sockaddrs, sizeof(tcp_ctx.get_accept_ex_sockaddrs), &len, NULL, NULL);
	ff_winsock_fatal_error_check(rv == 0, L"cannot obtain GetAcceptExSockaddrs() function");
	ff_assert(tcp_ctx.get_accept_ex_sockaddrs != NULL);
}

void ff_win_tcp_shutdown()
{
	int rv;

	rv = closesocket(tcp_ctx.aux_socket);
	ff_assert(rv == 0);

	rv = WSACleanup();
	ff_assert(rv == 0);
}

struct ff_arch_tcp_addr *ff_arch_tcp_addr_create()
{
	struct ff_arch_tcp_addr *addr;

	addr = (struct ff_arch_tcp_addr *) ff_malloc(sizeof(*addr));
		
	return addr;
}

void ff_arch_tcp_addr_delete(struct ff_arch_tcp_addr *addr)
{
	ff_free(addr);
}

int ff_arch_tcp_addr_resolve(struct ff_arch_tcp_addr *addr, const wchar_t *host, int port)
{
	struct threadpool_addr_resolve_data data;
	data.addr = addr;
	data.host = host;
	data.port = port;
	data.is_success = 0;
	ff_core_threadpool_execute(threadpool_addr_resolve_func, &data);

	return data.is_success;
}

struct ff_arch_tcp *ff_arch_tcp_create()
{
	HANDLE handle;
	struct ff_arch_tcp *tcp;

	tcp = (struct ff_arch_tcp *) ff_malloc(sizeof(*tcp));
	tcp->handle = create_tcp_socket();
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

int ff_arch_tcp_bind(struct ff_arch_tcp *tcp, const struct ff_arch_tcp_addr *addr)
{
	int rv;
	int is_success = 0;

	rv = bind(tcp->handle, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != SOCKET_ERROR)
	{
		is_success = 1;
	}

	return is_success;
}

void ff_arch_tcp_enable_listening_mode(struct ff_arch_tcp *tcp)
{
	int rv;

	rv = listen(tcp->handle, SOMAXCONN);
	ff_winsock_fatal_error_check(rv != SOCKET_ERROR, L"cannot enable listening mode for the tcp socket");
}

int ff_arch_tcp_connect(struct ff_arch_tcp *tcp, const struct ff_arch_tcp_addr *addr)
{
	int is_connected = 0;
	BOOL result;
	WSAOVERLAPPED overlapped;
	int bytes_transferred;
	struct sockaddr_in local_addr;
	int rv;

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(0);
	rv = bind(tcp->handle, (struct sockaddr *) &local_addr, sizeof(local_addr));
	ff_winsock_fatal_error_check(rv != SOCKET_ERROR, L"cannot bind arbitrary address");

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

struct ff_arch_tcp *ff_arch_tcp_accept(struct ff_arch_tcp *tcp, struct ff_arch_tcp_addr *remote_addr)
{
	WSAOVERLAPPED overlapped;
	struct ff_arch_tcp *remote_tcp;
	DWORD local_addr_len;
	DWORD remote_addr_len;
	DWORD bytes_read;
	BOOL result;
	size_t addr_buf_size;
	char *addr_buf;
	int bytes_transferred;
	int local_sockaddr_len;
	int remote_sockaddr_len;
	struct sockaddr *local_addr_ptr;
	struct sockaddr *remote_addr_ptr;

	local_addr_len = sizeof(remote_addr->addr) + 16;
	remote_addr_len = sizeof(remote_addr->addr) + 16;
	addr_buf_size = local_addr_len + remote_addr_len;
	addr_buf = (char *) ff_malloc(addr_buf_size);

	remote_tcp = ff_arch_tcp_create();
	memset(&overlapped, 0, sizeof(overlapped));
	result = tcp_ctx.accept_ex(tcp->handle, remote_tcp->handle, addr_buf, 0, local_addr_len, remote_addr_len, &bytes_read, &overlapped);
	if (result == FALSE)
	{
		int last_error;

		last_error = WSAGetLastError();
		if (last_error != ERROR_IO_PENDING)
		{
			ff_arch_tcp_delete(remote_tcp);
			remote_tcp = NULL;
			goto end;
		}
	}

	bytes_transferred = complete_overlapped_io(tcp, &overlapped, 0);
	if (bytes_transferred == -1)
	{
		ff_arch_tcp_delete(remote_tcp);
		remote_tcp = NULL;
		goto end;
	}

	tcp_ctx.get_accept_ex_sockaddrs(addr_buf, 0, local_addr_len, remote_addr_len,
		&local_addr_ptr, &local_sockaddr_len, &remote_addr_ptr, &remote_sockaddr_len);
	ff_assert(local_sockaddr_len == sizeof(remote_addr->addr));
	ff_assert(remote_sockaddr_len == sizeof(remote_addr->addr));
	memcpy(&remote_addr->addr, remote_addr_ptr, sizeof(remote_addr->addr));

end:
	ff_free(addr_buf);
	return remote_tcp;
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
