#include "private/ff_common.h"

#include "private/ff_udp.h"
#include "private/arch/ff_arch_udp.h"

struct ff_udp
{
	struct ff_arch_udp *udp;
};

struct ff_udp *ff_udp_create(int is_broadcast)
{
	struct ff_udp *udp;

	udp = (struct ff_udp *) ff_malloc(sizeof(*udp));
	udp->udp = ff_arch_udp_create(is_broadcast);

	return udp;
}

void ff_udp_delete(struct ff_udp *udp)
{
	ff_arch_udp_delete(udp->udp);
	ff_free(udp);
}

int ff_udp_bind(struct ff_udp *udp, const struct ff_arch_net_addr *addr)
{
	int is_success;

	is_success = ff_arch_udp_bind(udp->udp, addr);
	return is_success;
}

int ff_udp_read(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len, int timeout)
{
	int bytes_read;

	bytes_read = ff_arch_udp_read(udp->udp, peer_addr, buf, len, timeout);
	return bytes_read;
}

int ff_udp_write(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len, int timeout)
{
	int bytes_written;

	bytes_written = ff_arch_udp_write(udp->udp, addr, buf, len, timeout);
	return bytes_written;
}

void ff_udp_disconnect(struct ff_udp *udp)
{
	ff_arch_udp_disconnect(udp->udp);
}
