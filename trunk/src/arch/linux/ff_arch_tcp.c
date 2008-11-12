#include "private/ff_common.h"

#include "private/arch/ff_arch_tcp.h"
#include "ff_linux_net_addr.h"
#include "ff_linux_net.h"
#include "ff_linux_error_check.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

struct ff_arch_tcp
{
	int sd_rd;
	int sd_wr;
};

static struct ff_arch_tcp *create_tcp(int sd)
{
	struct ff_arch_tcp *tcp;
	int rv;
	int sd_rd, sd_wr;

	sd_rd = sd;
	rv = fcntl(sd_rd, F_SETFL, O_NONBLOCK);
	ff_linux_fatal_error_check(rv != -1, L"cannot set nonblocking mode for the TCP socket");

	sd_wr = dup(sd_rd);
	ff_linux_fatal_error_check(sd_wr != -1, L"cannot duplicate TCP socket");

	tcp = (struct ff_arch_tcp *) ff_malloc(sizeof(*tcp));
	tcp->sd_rd = sd_rd;
	tcp->sd_wr = sd_wr;

	return tcp;
}

struct ff_arch_tcp *ff_arch_tcp_create()
{
	struct ff_arch_tcp *tcp;
	int sd;

	sd = socket(PF_INET, SOCK_STREAM, 0);
	ff_linux_fatal_error_check(sd != -1, L"cannot create TCP socket");
	tcp = create_tcp(sd);

	return tcp;
}

void ff_arch_tcp_delete(struct ff_arch_tcp *tcp)
{
	int rv;

	rv = close(tcp->sd_rd);
	ff_assert(rv != -1);
	rv = close(tcp->sd_wr);
	ff_assert(rv != -1);
	ff_free(tcp);
}

enum ff_result ff_arch_tcp_bind(struct ff_arch_tcp *tcp, const struct ff_arch_net_addr *addr, int is_listening)
{
	int rv;
	enum ff_result result = FF_FAILURE;

	rv = bind(tcp->sd_rd, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv != -1)
	{
		if (is_listening)
		{
			rv = listen(tcp->sd_rd, SOMAXCONN);
			ff_linux_fatal_error_check(rv != -1, L"error in the listen()");
		}
		result = FF_SUCCESS;
	}
	else
	{
		ff_log_debug(L"cannot bind the sd_rd=%d to the addr=%p. errno=%d", tcp->sd_rd, addr, errno);
	}

	return result;
}

enum ff_result ff_arch_tcp_connect(struct ff_arch_tcp *tcp, const struct ff_arch_net_addr *addr)
{
	int rv;
	enum ff_result result = FF_SUCCESS;

again:
	rv = connect(tcp->sd_wr, (struct sockaddr *) &addr->addr, sizeof(addr->addr));
	if (rv == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}

		result = FF_FAILURE;
		if (errno == EINPROGRESS)
		{
			int err;
			socklen_t optlen = sizeof(err);

			ff_linux_net_wait_for_io(tcp->sd_wr, FF_LINUX_NET_IO_WRITE);
			rv = getsockopt(tcp->sd_wr, SOL_SOCKET, SO_ERROR, &err, &optlen);
			ff_assert(rv != -1);
			ff_assert(optlen == sizeof(err));
			if (err == 0)
			{
				result = FF_SUCCESS;
			}
			else
			{
				ff_log_debug(L"error while connecting sd_wr=%d to the addr=%p. err=%d", tcp->sd_wr, addr, err);
			}
		}
		else
		{
			ff_log_debug(L"cannot connect the sd_wr=%d to the addr=%p. errno=%d", tcp->sd_wr, addr, errno);
		}
	}

	return result;
}

struct ff_arch_tcp *ff_arch_tcp_accept(struct ff_arch_tcp *tcp, struct ff_arch_net_addr *remote_addr)
{
	int accepted_sd;
	socklen_t addrlen = sizeof(remote_addr->addr);
	struct ff_arch_tcp *accepted_tcp = NULL;

again:
	accepted_sd = accept(tcp->sd_rd, (struct sockaddr *) &remote_addr->addr, &addrlen);
	if (accepted_sd == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			ff_linux_net_wait_for_io(tcp->sd_rd, FF_LINUX_NET_IO_READ);
			goto again;
		}
		ff_log_debug(L"cannot accept connection to the sd_rd=%d, remote_addr=%p. errno=%d", tcp->sd_rd, remote_addr, errno);
	}
	else
	{
		ff_assert(addrlen == sizeof(remote_addr->addr));
		accepted_tcp = create_tcp(accepted_sd);
	}

	return accepted_tcp;
}

int ff_arch_tcp_read(struct ff_arch_tcp *tcp, void *buf, int len)
{
	ssize_t bytes_read;
	int bytes_read_int;

again:
	bytes_read = recv(tcp->sd_rd, buf, len, 0);
	if (bytes_read == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN)
		{
			ff_linux_net_wait_for_io(tcp->sd_rd, FF_LINUX_NET_IO_READ);
			goto again;
		}
		ff_log_debug(L"cannot read from the sd_rd=%d to the buf=%p, len=%d. errno=%d", tcp->sd_rd, buf, len, errno);
	}

	bytes_read_int = (int) bytes_read;
	return bytes_read_int;
}

int ff_arch_tcp_write(struct ff_arch_tcp *tcp, const void *buf, int len)
{
	ssize_t bytes_written;
	int bytes_written_int;

again:
	bytes_written = send(tcp->sd_wr, buf, len, 0);
	if (bytes_written == -1)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			ff_linux_net_wait_for_io(tcp->sd_wr, FF_LINUX_NET_IO_WRITE);
			goto again;
		}
		ff_log_debug(L"cannot write to the sd_wr=%d from the buf=%p, len=%d. errno=%d", tcp->sd_wr, buf, len, errno);
	}

	bytes_written_int = (int) bytes_written;
	return bytes_written_int;
}

void ff_arch_tcp_disconnect(struct ff_arch_tcp *tcp)
{
	int rv;

	rv = shutdown(tcp->sd_rd, SHUT_RD);
	if (rv != -1)
	{
		rv = shutdown(tcp->sd_wr, SHUT_WR);
		if (rv == -1)
		{
			/* server socket returns ENOTCONN error when shutting down the tcp->sd_wr */
			ff_assert(errno == ENOTCONN);
		}
	}
	else
	{
		/* socket already has been shutdowned */
		ff_assert(errno == ENOTCONN);
		ff_log_debug(L"the tcp=%p was already shutdowned", tcp);
	}
}
