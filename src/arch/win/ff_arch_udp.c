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

enum ff_result ff_arch_udp_bind(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr)
{
	int rv;
	enum ff_result result = FF_FAILURE;

	if (!udp->is_working)
	{
		ff_log_debug(L"udp=%p was already disconnected, so it cannot be bound to the addr=%p", udp, addr);
		goto end;
	}

	rv = bind(udp->handle, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != SOCKET_ERROR)
	{
		result = FF_SUCCESS;
	}
	else
	{
		int last_error;

		last_error = WSAGetLastError();
		ff_log_debug(L"cannot bind the udp=%p to the addr=%p. WSAGetLastError()=%d", udp, addr, last_error);
	}

end:
	return result;
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
		ff_log_debug(L"udp=%p was already disconnected, so it cannot be used for reading to the buf=%p, len=%d, peer_addr=%p", udp, buf, len, peer_addr);
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
			ff_log_debug(L"error while reading from the udp=%p into the buf=%p, len=%d, peer_addr=%p. WSAGetLastError()=%d", udp, buf, len, peer_addr, last_error);
			goto end;
		}
	}
	ff_assert(peer_addr_len == sizeof(peer_addr->addr));

	int_bytes_read = ff_win_net_complete_overlapped_io(udp->handle, &overlapped);
	if (int_bytes_read == -1)
	{
		ff_log_debug(L"error while reading from the udp=%p into the buf=%p, len=%p, peer_addr=%p using overlapped=%p. See previous messages for more info",
			udp, buf, len, peer_addr, &overlapped);
	}

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
		ff_log_debug(L"udp=%p was already disconnected, so it cannot be used for writing from the buf=%p, len=%d to the addr=%p", udp, buf, len, addr);
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
			ff_log_debug(L"error while writing to the udp=%p from the buf=%p, len=%d to the addr=%p. WSAGetLastError()=%d", udp, buf, len, addr, last_error);
			goto end;
		}
	}

	int_bytes_written = ff_win_net_complete_overlapped_io(udp->handle, &overlapped);
	if (int_bytes_written == -1)
	{
		ff_log_debug(L"error while writing to the udp=%p from the buf=%p, len=%d to the addr=%p using overlapped=%p. See previous messages for more info",
			udp, buf, len, addr, &overlapped);
	}

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
	else
	{
		ff_log_debug(L"udp=%p was already disconnected, so it won't be disconnected again", udp);
	}
}
