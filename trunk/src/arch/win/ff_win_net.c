#include "ff_win_stdafx.h"

#include "ff_win_net.h"
#include "private/arch/ff_arch_completion_port.h"
#include "ff_win_completion_port.h"
#include "private/ff_core.h"
#include "private/ff_fiber.h"

struct net_data
{
	struct ff_arch_completion_port *completion_port;
	LPFN_CONNECTEX connect_ex;
	LPFN_ACCEPTEX accept_ex;
	LPFN_GETACCEPTEXSOCKADDRS get_accept_ex_sockaddrs;
};

static struct net_data net_ctx;

void ff_win_net_initialize(struct ff_arch_completion_port *completion_port)
{
	int rv;
	WORD version;
	struct WSAData wsa;
	SOCKET aux_socket;
	DWORD len;
	GUID connect_ex_guid = WSAID_CONNECTEX;
	GUID accept_ex_guid = WSAID_ACCEPTEX;
	GUID get_accept_ex_sockaddrs_guid = WSAID_GETACCEPTEXSOCKADDRS;

	net_ctx.completion_port = completion_port;

	version = MAKEWORD(2, 2);
	rv = WSAStartup(version, &wsa);
	ff_winsock_fatal_error_check(rv == 0, L"cannot initialize winsock");

	aux_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ff_winsock_fatal_error_check(aux_socket != INVALID_SOCKET, L"cannot create tcp socket");

	net_ctx.connect_ex = NULL;
	rv = WSAIoctl(aux_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &connect_ex_guid, sizeof(connect_ex_guid),
		&net_ctx.connect_ex, sizeof(net_ctx.connect_ex), &len, NULL, NULL);
	ff_winsock_fatal_error_check(rv == 0, L"cannot obtain ConnectEx() function");
	ff_assert(net_ctx.connect_ex != NULL);

	net_ctx.accept_ex = NULL;
	rv = WSAIoctl(aux_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &accept_ex_guid, sizeof(accept_ex_guid),
		&net_ctx.accept_ex, sizeof(net_ctx.accept_ex), &len, NULL, NULL);
	ff_winsock_fatal_error_check(rv == 0, L"cannot obtain AcceptEx() function");
	ff_assert(net_ctx.accept_ex != NULL);

	net_ctx.get_accept_ex_sockaddrs = NULL;
	rv = WSAIoctl(aux_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &get_accept_ex_sockaddrs_guid, sizeof(get_accept_ex_sockaddrs_guid),
		&net_ctx.get_accept_ex_sockaddrs, sizeof(net_ctx.get_accept_ex_sockaddrs), &len, NULL, NULL);
	ff_winsock_fatal_error_check(rv == 0, L"cannot obtain GetAcceptExSockaddrs() function");
	ff_assert(net_ctx.get_accept_ex_sockaddrs != NULL);

	rv = closesocket(aux_socket);
	ff_assert(rv == 0);
}

void ff_win_net_shutdown()
{
	int rv;

	rv = WSACleanup();
	ff_assert(rv == 0);
}

void ff_win_net_register_socket(SOCKET socket)
{
	ff_win_completion_port_register_handle(net_ctx.completion_port, (HANDLE) socket);
}

int ff_win_net_complete_overlapped_io(SOCKET socket, WSAOVERLAPPED *overlapped)
{
	struct ff_fiber *current_fiber;
	int int_bytes_transferred = -1;
	BOOL result;
	DWORD flags;
	DWORD bytes_transferred;

	current_fiber = ff_fiber_get_current();
	ff_win_completion_port_register_overlapped_data(net_ctx.completion_port, overlapped, current_fiber);
	ff_core_yield_fiber();
	ff_win_completion_port_deregister_overlapped_data(net_ctx.completion_port, overlapped);

	result = WSAGetOverlappedResult(socket, overlapped, &bytes_transferred, FALSE, &flags);
	if (result != FALSE)
	{
		int_bytes_transferred = (int) bytes_transferred;
	}

	return int_bytes_transferred;
}

enum ff_result ff_win_net_connect(SOCKET socket, const struct sockaddr_in *addr)
{
	BOOL is_connected;
	WSAOVERLAPPED overlapped;
	int bytes_transferred;
	struct sockaddr_in local_addr;
	int rv;
	enum ff_result result = FF_FAILURE;

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(0);
	rv = bind(socket, (struct sockaddr *) &local_addr, sizeof(local_addr));
	ff_winsock_fatal_error_check(rv != SOCKET_ERROR, L"cannot bind arbitrary address");

	memset(&overlapped, 0, sizeof(overlapped));
	is_connected = net_ctx.connect_ex(socket, (struct sockaddr *) addr, sizeof(*addr), NULL, 0, NULL, &overlapped);
	if (is_connected == FALSE)
	{
		int last_error;

		last_error = WSAGetLastError();
		if (last_error != WSA_IO_PENDING)
		{
			goto end;
		}
	}

	bytes_transferred = ff_win_net_complete_overlapped_io(socket, &overlapped);
	if (bytes_transferred == -1)
	{
		goto end;
	}
	result = FF_SUCCESS;

end:
	return result;
}

enum ff_result ff_win_net_accept(SOCKET listen_socket, SOCKET remote_socket, struct sockaddr_in *remote_addr)
{
	WSAOVERLAPPED overlapped;
	DWORD local_addr_len;
	DWORD remote_addr_len;
	DWORD bytes_read;
	BOOL is_accepted;
	size_t addr_buf_size;
	char *addr_buf;
	int bytes_transferred;
	int local_sockaddr_len;
	int remote_sockaddr_len;
	struct sockaddr *local_addr_ptr;
	struct sockaddr *remote_addr_ptr;
	enum ff_result result = FF_FAILURE;

	local_addr_len = sizeof(*remote_addr) + 16;
	remote_addr_len = sizeof(*remote_addr) + 16;
	addr_buf_size = local_addr_len + remote_addr_len;
	addr_buf = (char *) ff_calloc(addr_buf_size, sizeof(addr_buf[0]));

	memset(&overlapped, 0, sizeof(overlapped));
	is_accepted = net_ctx.accept_ex(listen_socket, remote_socket, addr_buf, 0, local_addr_len, remote_addr_len, &bytes_read, &overlapped);
	if (is_accepted == FALSE)
	{
		int last_error;

		last_error = WSAGetLastError();
		if (last_error != ERROR_IO_PENDING)
		{
			goto end;
		}
	}

	bytes_transferred = ff_win_net_complete_overlapped_io(listen_socket, &overlapped);
	if (bytes_transferred == -1)
	{
		goto end;
	}

	net_ctx.get_accept_ex_sockaddrs(addr_buf, 0, local_addr_len, remote_addr_len,
		&local_addr_ptr, &local_sockaddr_len, &remote_addr_ptr, &remote_sockaddr_len);
	ff_assert(local_sockaddr_len == sizeof(*remote_addr));
	ff_assert(remote_sockaddr_len == sizeof(*remote_addr));
	memcpy(remote_addr, remote_addr_ptr, sizeof(*remote_addr));

	result = FF_SUCCESS;

end:
	ff_free(addr_buf);
	return result;
}
