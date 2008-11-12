#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_tcp.h"
#include "ff_win_net.h"
#include "ff_win_net_addr.h"

struct ff_arch_tcp
{
	SOCKET handle;
	int is_working;
};

struct ff_arch_tcp *ff_arch_tcp_create()
{
	struct ff_arch_tcp *tcp;

	tcp = (struct ff_arch_tcp *) ff_malloc(sizeof(*tcp));
	tcp->handle = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ff_winsock_fatal_error_check(tcp->handle != INVALID_SOCKET, L"cannot create tcp socket");
	tcp->is_working = 1;

	ff_win_net_register_socket(tcp->handle);

	return tcp;
}

void ff_arch_tcp_delete(struct ff_arch_tcp *tcp)
{
	int rv;

	rv = closesocket(tcp->handle);
	ff_assert(rv == 0);
	ff_free(tcp);
}

enum ff_result ff_arch_tcp_bind(struct ff_arch_tcp *tcp, const struct ff_arch_net_addr *addr, int is_listening)
{
	int rv;
	enum ff_result result = FF_FAILURE;

	if (!tcp->is_working)
	{
		ff_log_debug(L"tcp=%p was disconnected, so it cannot be bound again to the addr=%p", tcp, addr);
		goto end;
	}

	rv = bind(tcp->handle, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != SOCKET_ERROR)
	{
		if (is_listening)
		{
			rv = listen(tcp->handle, SOMAXCONN);
			ff_winsock_fatal_error_check(rv != SOCKET_ERROR, L"cannot enable listening mode for the tcp socket");
		}
		result = FF_SUCCESS;
	}
	else
	{
		int last_error;

		last_error = WSAGetLastError();
		ff_log_debug(L"cannot bind the addr=%p on the tcp=%p. WSAGetLastError()=%d", addr, tcp, last_error);
	}

end:
	return result;
}

enum ff_result ff_arch_tcp_connect(struct ff_arch_tcp *tcp, const struct ff_arch_net_addr *addr)
{
	enum ff_result result = FF_FAILURE;
	
	if (tcp->is_working)
	{
		result = ff_win_net_connect(tcp->handle, &addr->addr);
		if (result != FF_SUCCESS)
		{
			ff_log_debug(L"cannot establish connection for the tcp=%p on the address=%p. See previous messages for more info", tcp, addr);
		}
	}
	else
	{
		ff_log_debug(L"tcp=%p was disconnected, so it cannot be connected again to the addr=%p", tcp, addr);
	}
	return result;
}

struct ff_arch_tcp *ff_arch_tcp_accept(struct ff_arch_tcp *tcp, struct ff_arch_net_addr *remote_addr)
{
	struct ff_arch_tcp *remote_tcp = NULL;
	enum ff_result result;

	if (!tcp->is_working)
	{
		ff_log_debug(L"tcp=%p was disconnected, so it cannot be used for accepting. remote_addr=%p", tcp, remote_addr);
		goto end;
	}

	remote_tcp = ff_arch_tcp_create();
	result = ff_win_net_accept(tcp->handle, remote_tcp->handle, &remote_addr->addr);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while accepting connections on the tcp=%p, remote_addr=%p. See previous messages for more info", tcp, remote_addr);
		ff_arch_tcp_delete(remote_tcp);
		remote_tcp = NULL;
	}

end:
	return remote_tcp;
}

int ff_arch_tcp_read(struct ff_arch_tcp *tcp, void *buf, int len)
{
	int rv;
	WSAOVERLAPPED overlapped;
	WSABUF wsa_buf;
	int int_bytes_read = -1;
	DWORD flags = 0;

	ff_assert(len >= 0);

	if (!tcp->is_working)
	{
		ff_log_debug(L"tcp=%p was disconnected, so it cannot be used for reading to the buf=%p, len=%d", tcp, buf, len);
		goto end;
	}

	wsa_buf.len = (u_long) len;
	wsa_buf.buf = (char *) buf;
	memset(&overlapped, 0, sizeof(overlapped));
	rv = WSARecv(tcp->handle, &wsa_buf, 1, NULL, &flags, &overlapped, NULL);
	if (rv != 0)
	{
		int last_error;
		
		last_error = WSAGetLastError();
		if (last_error != WSA_IO_PENDING)
		{
			ff_log_debug(L"error while reading data from the tcp=%p to the buf=%p, len=%d. WSAGetLastError()=%d", tcp, buf, len, last_error);
			goto end;
		}
	}

	int_bytes_read = ff_win_net_complete_overlapped_io(tcp->handle, &overlapped);
	if (int_bytes_read == -1)
	{
		ff_log_debug(L"error while reading data from the tcp=%p to the buf=%p, len=%d using overlapped=%p. See previous messages for more info", tcp, buf, len, &overlapped);
	}

end:
	return int_bytes_read;
}

int ff_arch_tcp_write(struct ff_arch_tcp *tcp, const void *buf, int len)
{
	int rv;
	WSAOVERLAPPED overlapped;
	WSABUF wsa_buf;
	int int_bytes_written = -1;
	DWORD flags = 0;

	ff_assert(len >= 0);

	if (!tcp->is_working)
	{
		ff_log_debug(L"tcp=%p was disconnected, so it cannot be used for writing from the buf=%p, len=%d", tcp, buf, len);
		goto end;
	}

	wsa_buf.len = len;
	wsa_buf.buf = (char *) buf;
	memset(&overlapped, 0, sizeof(overlapped));
	rv = WSASend(tcp->handle, &wsa_buf, 1, NULL, flags, &overlapped, NULL);
	if (rv != 0)
	{
		int last_error;
		
		last_error = WSAGetLastError();
		if (last_error != WSA_IO_PENDING)
		{
			ff_log_debug(L"error while writing data to the tcp=%p from the buf=%p, len=%d. WSAGetLastError()=%d", tcp, buf, len, last_error);
			goto end;
		}
	}

	int_bytes_written = ff_win_net_complete_overlapped_io(tcp->handle, &overlapped);
	if (int_bytes_written == -1)
	{
		ff_log_debug(L"error while reading data to the tcp=%p from the buf=%p, len=%d using overlapped=%p. See previous messages for more info", tcp, buf, len, &overlapped);
	}

end:
	return int_bytes_written;
}

void ff_arch_tcp_disconnect(struct ff_arch_tcp *tcp)
{
	if (tcp->is_working)
	{
		BOOL result;

		tcp->is_working = 0;
		result = CancelIo((HANDLE) tcp->handle);
		ff_assert(result != FALSE);
	}
	else
	{
		ff_log_debug(L"the tcp=%p was already disconnected, so it won't be disconnected again", tcp);
	}
}
