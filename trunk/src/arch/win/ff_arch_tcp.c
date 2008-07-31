#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_tcp.h"
#include "ff_win_net.h"
#include "ff_win_net_addr.h"

struct ff_arch_tcp
{
	SOCKET handle;
	int is_connected;
};

struct ff_arch_tcp *ff_arch_tcp_create()
{
	struct ff_arch_tcp *tcp;

	tcp = (struct ff_arch_tcp *) ff_malloc(sizeof(*tcp));
	tcp->handle = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ff_winsock_fatal_error_check(tcp->handle != INVALID_SOCKET, L"cannot create tcp socket");
	tcp->is_connected = 0;

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

int ff_arch_tcp_bind(struct ff_arch_tcp *tcp, const struct ff_arch_net_addr *addr, int is_listening)
{
	int rv;
	int is_success = 0;

	rv = bind(tcp->handle, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != SOCKET_ERROR)
	{
		if (is_listening)
		{
			rv = listen(tcp->handle, SOMAXCONN);
			ff_winsock_fatal_error_check(rv != SOCKET_ERROR, L"cannot enable listening mode for the tcp socket");
		}
		is_success = 1;
	}

	return is_success;
}

int ff_arch_tcp_connect(struct ff_arch_tcp *tcp, const struct ff_arch_net_addr *addr)
{
	tcp->is_connected = ff_win_net_connect(tcp->handle, &addr->addr);
	return tcp->is_connected;
}

struct ff_arch_tcp *ff_arch_tcp_accept(struct ff_arch_tcp *tcp, struct ff_arch_net_addr *remote_addr)
{
	struct ff_arch_tcp *remote_tcp;
	int is_success;

	remote_tcp = ff_arch_tcp_create();
	is_success = ff_win_net_accept(tcp->handle, remote_tcp->handle, &remote_addr->addr);
	if (!is_success)
	{
		ff_arch_tcp_delete(remote_tcp);
		remote_tcp = NULL;
	}
	else
	{
		remote_tcp->is_connected = 1;
	}

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

	if (!tcp->is_connected)
	{
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
			goto end;
		}
	}

	int_bytes_read = ff_win_net_complete_overlapped_io(tcp->handle, &overlapped);

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

	if (!tcp->is_connected)
	{
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
			goto end;
		}
	}

	int_bytes_written = ff_win_net_complete_overlapped_io(tcp->handle, &overlapped);

end:
	return int_bytes_written;
}

void ff_arch_tcp_disconnect(struct ff_arch_tcp *tcp)
{
	BOOL result;

	ff_assert(tcp->is_connected);

	tcp->is_connected = 0;
	result = CancelIo((HANDLE) tcp->handle);
	ff_assert(result != FALSE);
}
