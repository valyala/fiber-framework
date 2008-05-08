#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_udp.h"
#include "private/ff_core.h"
#include "ff_win_net_addr.h"

struct ff_arch_udp
{
	SOCKET handle;
};

struct threadpool_read_udp_data
{
	struct ff_arch_udp *udp;
	struct ff_arch_net_addr *peer_addr;
	void *buf;
	int len;
	int timeout;
	int bytes_read;
};

struct threadpool_write_udp_data
{
	struct ff_arch_udp *udp;
	const struct ff_arch_net_addr *addr;
	const void *buf;
	int len;
	int timeout;
	int bytes_written;
};

static void threadpool_read_udp_func(void *ctx)
{
	struct threadpool_read_udp_data *data;
	int rv;
	int peer_addr_len;

	data = (struct threadpool_read_udp_data *) ctx;
	rv = setsockopt(data->udp->handle, SOL_SOCKET, SO_RCVTIMEO, (char *) &data->timeout, sizeof(data->timeout));
	ff_assert(rv == 0);

	peer_addr_len = sizeof(data->peer_addr->addr);
	data->bytes_read = recvfrom(data->udp->handle, (char *) data->buf, data->len, 0, (struct sockaddr *) &data->peer_addr->addr, &peer_addr_len);
	ff_assert(peer_addr_len == sizeof(data->peer_addr->addr));
	if (data->bytes_read == SOCKET_ERROR)
	{
		data->bytes_read = -1;
	}
}

static void threadpool_write_udp_func(void *ctx)
{
	struct threadpool_write_udp_data *data;
	int rv;

	data = (struct threadpool_write_udp_data *) ctx;
	rv = setsockopt(data->udp->handle, SOL_SOCKET, SO_SNDTIMEO, (char *) &data->timeout, sizeof(data->timeout));
	ff_assert(rv == 0);

	data->bytes_written = sendto(data->udp->handle, (char *) data->buf, data->len, 0, (struct sockaddr *) &data->addr->addr, sizeof(data->addr->addr));
	if (data->bytes_written == SOCKET_ERROR)
	{
		data->bytes_written = -1;
	}
}

struct ff_arch_udp *ff_arch_udp_create(int is_broadcast)
{
	struct ff_arch_udp *udp;

	udp = (struct ff_arch_udp *) ff_malloc(sizeof(*udp));
	udp->handle = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ff_winsock_fatal_error_check(udp->handle != INVALID_SOCKET, L"cannot create udp socket");

	if (is_broadcast)
	{
		BOOL opt_val;
		int rv;
		
		opt_val = TRUE;
		rv = setsockopt(udp->handle, SOL_SOCKET, SO_BROADCAST, (char *) &opt_val, sizeof(opt_val));
		ff_assert(rv == 0);
	}

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

	rv = bind(udp->handle, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != SOCKET_ERROR)
	{
		is_success = 1;
	}

	return is_success;
}

int ff_arch_udp_read(struct ff_arch_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len, int timeout)
{
	struct threadpool_read_udp_data data;

	data.udp = udp;
	data.peer_addr = peer_addr;
	data.buf = buf;
	data.len = len;
	data.timeout = timeout;
	data.bytes_read = -1;
	ff_core_threadpool_execute(threadpool_read_udp_func, &data);

	return data.bytes_read;
}

int ff_arch_udp_write(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len, int timeout)
{
	struct threadpool_write_udp_data data;

	data.udp = udp;
	data.addr = addr;
	data.buf = buf;
	data.len = len;
	data.timeout = timeout;
	data.bytes_written = -1;
	ff_core_threadpool_execute(threadpool_write_udp_func, &data);

	return data.bytes_written;
}

void ff_arch_udp_disconnect(struct ff_arch_udp *udp)
{
	BOOL result;
	int rv;

	rv = shutdown(udp->handle, SD_BOTH);
	ff_assert(rv == 0);

	result = CancelIo((HANDLE) udp->handle);
	ff_assert(result != FALSE);
}
