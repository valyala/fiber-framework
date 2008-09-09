#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_udp.h"
#include "ff_win_net.h"
#include "ff_win_net_addr.h"

struct ff_arch_udp
{
	SOCKET handle;
	int is_working;
};

struct ff_arch_udp *ff_arch_udp_create(int is_broadcast)
{
	struct ff_arch_udp *udp;

	udp = (struct ff_arch_udp *) ff_malloc(sizeof(*udp));
	udp->handle = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ff_winsock_fatal_error_check(udp->handle != INVALID_SOCKET, L"cannot create udp socket");
	udp->is_working = 1;

	if (is_broadcast)
	{
		BOOL opt_val;
		int rv;
		
		opt_val = TRUE;
		rv = setsockopt(udp->handle, SOL_SOCKET, SO_BROADCAST, (char *) &opt_val, sizeof(opt_val));
		ff_assert(rv == 0);
	}

	ff_win_net_register_socket(udp->handle);

	return udp;
}

void ff_arch_udp_delete(struct ff_arch_udp *udp)
{
	int rv;

	rv = closesocket(udp->handle);
	ff_assert(rv == 0);
	ff_free(udp);
}

int ff_arch_udp_bind(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr)
{
	int rv;
	int is_success = 0;

	if (!udp->is_working)
	{
		goto end;
	}

	rv = bind(udp->handle, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != SOCKET_ERROR)
	{
		is_success = 1;
	}

end:
	return is_success;
}

int ff_arch_udp_read(struct ff_arch_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len)
{
	int rv;
	WSAOVERLAPPED overlapped;
	WSABUF wsa_buf;
	int int_bytes_read = -1;
	DWORD flags = 0;
	INT peer_addr_len;

	ff_assert(len >= 0);

	if (!udp->is_working)
	{
		goto end;
	}

	wsa_buf.len = (u_long) len;
	wsa_buf.buf = (char *) buf;
	peer_addr_len = sizeof(peer_addr->addr);
	memset(&overlapped, 0, sizeof(overlapped));
	rv = WSARecvFrom(udp->handle, &wsa_buf, 1, NULL, &flags, (struct sockaddr *) &peer_addr->addr, &peer_addr_len, &overlapped, NULL);
	if (rv != 0)
	{
		int last_error;
		
		last_error = WSAGetLastError();
		if (last_error != WSA_IO_PENDING)
		{
			goto end;
		}
	}
	ff_assert(peer_addr_len == sizeof(peer_addr->addr));

	int_bytes_read = ff_win_net_complete_overlapped_io(udp->handle, &overlapped);

end:
	return int_bytes_read;
}

int ff_arch_udp_write(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len)
{
	int rv;
	WSAOVERLAPPED overlapped;
	WSABUF wsa_buf;
	int int_bytes_written = -1;
	DWORD flags = 0;
	INT addr_len;

	ff_assert(len >= 0);

	if (!udp->is_working)
	{
		goto end;
	}

	wsa_buf.len = len;
	wsa_buf.buf = (char *) buf;
	addr_len = sizeof(addr->addr);
	memset(&overlapped, 0, sizeof(overlapped));
	rv = WSASendTo(udp->handle, &wsa_buf, 1, NULL, flags, (struct sockaddr *) &addr->addr, addr_len, &overlapped, NULL);
	if (rv != 0)
	{
		int last_error;
		
		last_error = WSAGetLastError();
		if (last_error != WSA_IO_PENDING)
		{
			goto end;
		}
	}

	int_bytes_written = ff_win_net_complete_overlapped_io(udp->handle, &overlapped);

end:
	return int_bytes_written;
}

void ff_arch_udp_disconnect(struct ff_arch_udp *udp)
{
	if (udp->is_working)
	{
		BOOL result;

		udp->is_working = 0;
		result = CancelIo((HANDLE) udp->handle);
		ff_assert(result != FALSE);
	}
}
