#include "private/ff_common.h"

#include "private/arch/ff_arch_udp.h"
#include "ff_linux_net_addr.h"
#include "ff_linux_error_check.h"
#include "ff_linux_net.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

struct ff_arch_udp
{
	int sd_rd;
	int sd_wr;
};

static struct ff_arch_udp *create_udp(int sd)
{
	struct ff_arch_udp *udp;
	int rv;
	int sd_rd, sd_wr;

	sd_rd = sd;
	rv = fcntl(sd_rd, F_SETFL, O_NONBLOCK);
	ff_linux_fatal_error_check(rv != -1, L"cannot set nonblocking mode for the UDP socket");

	sd_wr = dup(sd_rd);
	ff_linux_fatal_error_check(sd_wr != -1, L"cannot duplicate UDP socket");

	udp = (struct ff_arch_udp *) ff_malloc(sizeof(*udp));
	udp->sd_rd = sd_rd;
	udp->sd_wr = sd_wr;

	return udp;
}

struct ff_arch_udp *ff_arch_udp_create(int is_broadcast)
{
	struct ff_arch_udp *udp;
	int sd;

	sd = socket(PF_INET, SOCK_DGRAM, 0);
	ff_linux_fatal_error_check(sd != -1, L"cannot create UDP socket");
	udp = create_udp(sd);

	return udp;
}

void ff_arch_udp_delete(struct ff_arch_udp *udp)
{
	int rv;

	rv = close(udp->sd_rd);
	assert(rv != -1);
	rv = close(udp->sd_wr);
	assert(rv != -1);
	ff_free(udp);
}

int ff_arch_udp_bind(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr)
{
	int rv;
	int is_success = 0;

	rv = bind(udp->sd_rd, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != -1)
	{
		is_success = 1;
	}
	return is_success;
}

int ff_arch_udp_read(struct ff_arch_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len)
{
	ssize_t bytes_read;
	int bytes_read_int;
	socklen_t addrlen = sizeof(peer_addr->addr);

again:
	bytes_read = recvfrom(udp->sd_rd, buf, len, 0, (struct sockaddr *) &peer_addr->addr, &addrlen);
	if (bytes_read == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN)
		{
			ff_linux_net_wait_for_io(udp->sd_rd, FF_LINUX_NET_IO_READ);
			goto again;
		}
	}
	else
	{
		ff_assert(addrlen == sizeof(peer_addr->addr));
	}

	bytes_read_int = (int) bytes_read;
	return bytes_read_int;
}

int ff_arch_udp_write(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len)
{
	ssize_t bytes_written;
	int bytes_written_int;

again:
	bytes_written = sendto(udp->sd_wr, buf, len, 0, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (bytes_written == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			ff_linux_net_wait_for_io(udp->sd_wr, FF_LINUX_NET_IO_WRITE);
			goto again;
		}
	}

	bytes_written_int = (int) bytes_written;
	return bytes_written_int;
}

void ff_arch_udp_disconnect(struct ff_arch_udp *udp)
{
	int rv;

	rv = shutdown(udp->sd_rd, SHUT_RDWR);
	ff_assert(rv != -1);
}
