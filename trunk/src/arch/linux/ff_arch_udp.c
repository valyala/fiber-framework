#include "private/ff_common.h"

#include "private/arch/ff_arch_udp.h"
#include "private/ff_fiber.h"
#include "ff_linux_net_addr.h"
#include "ff_linux_error_check.h"
#include "ff_linux_net.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

struct ff_arch_udp
{
	struct ff_fiber *reader_fiber;
	struct ff_fiber *writer_fiber;
	int is_working;
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
	udp->reader_fiber = NULL;
	udp->writer_fiber = NULL;
	udp->is_working = 1;
	udp->sd_rd = sd_rd;
	udp->sd_wr = sd_wr;

	return udp;
}

static void shutdown_udp(struct ff_arch_udp *udp)
{
	int rv;

	ff_assert(udp->is_working);

	rv = close(udp->sd_rd);
	ff_assert(rv != -1);
	rv = close(udp->sd_wr);
	ff_assert(rv != -1);
}

struct ff_arch_udp *ff_arch_udp_create(int is_broadcast)
{
	struct ff_arch_udp *udp;
	int sd;

	sd = socket(PF_INET, SOCK_DGRAM, 0);
	ff_linux_fatal_error_check(sd != -1, L"cannot create UDP socket");
	udp = create_udp(sd);

	if (is_broadcast)
	{
		int opt_val;
		int rv;
		
		opt_val = 1;
		rv = setsockopt(udp->sd_wr, SOL_SOCKET, SO_BROADCAST, &opt_val, sizeof(opt_val));
		ff_assert(rv != -1);
	}

	return udp;
}

void ff_arch_udp_delete(struct ff_arch_udp *udp)
{
	ff_assert(udp->reader_fiber == NULL);
	ff_assert(udp->writer_fiber == NULL);

	if (udp->is_working)
	{
		shutdown_udp(udp);
	}
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

	rv = bind(udp->sd_rd, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != -1)
	{
		is_success = 1;
	}

end:
	return is_success;
}

int ff_arch_udp_read(struct ff_arch_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len)
{
	ssize_t bytes_read = -1;
	int bytes_read_int;
	socklen_t addrlen = sizeof(peer_addr->addr);
	struct ff_fiber *current_fiber;

	current_fiber = ff_fiber_get_current();
	ff_assert(current_fiber != NULL);
again:
	ff_assert(udp->reader_fiber == NULL);
	if (!udp->is_working)
	{
		goto end;
	}
	bytes_read = recvfrom(udp->sd_rd, buf, len, 0, (struct sockaddr *) &peer_addr->addr, &addrlen);
	if (bytes_read == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN)
		{
			udp->reader_fiber = current_fiber;
			ff_linux_net_wait_for_io(udp->sd_rd, FF_LINUX_NET_IO_READ);
			udp->reader_fiber = NULL;
			goto again;
		}
	}
	else
	{
		ff_assert(addrlen == sizeof(peer_addr->addr));
	}

end:
	bytes_read_int = (int) bytes_read;
	return bytes_read_int;
}

int ff_arch_udp_write(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len)
{
	ssize_t bytes_written = -1;
	int bytes_written_int;
	struct ff_fiber *current_fiber;

	current_fiber = ff_fiber_get_current();
	ff_assert(current_fiber != NULL);
again:
	ff_assert(udp->writer_fiber == NULL);
	if (!udp->is_working)
	{
		goto end;
	}
	bytes_written = sendto(udp->sd_wr, buf, len, 0, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (bytes_written == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			udp->writer_fiber = current_fiber;
			ff_linux_net_wait_for_io(udp->sd_wr, FF_LINUX_NET_IO_WRITE);
			udp->writer_fiber = NULL;
			goto again;
		}
	}

end:
	bytes_written_int = (int) bytes_written;
	return bytes_written_int;
}

void ff_arch_udp_disconnect(struct ff_arch_udp *udp)
{
	if (udp->is_working)
	{
		shutdown_udp(udp);
		udp->is_working = 0;
		if (udp->reader_fiber != NULL)
		{
			ff_linux_net_wakeup_fiber(udp->reader_fiber);
		}
		if (udp->writer_fiber != NULL)
		{
			ff_linux_net_wakeup_fiber(udp->writer_fiber);
		}
	}
}
