#include "private/arch/ff_arch_udp.h"
#include "ff_linux_net_addr.h"

struct ff_arch_udp
{
	int sd;
};

struct ff_arch_udp *ff_arch_udp_create(int is_broadcast)
{
}

void ff_arch_udp_delete(struct ff_arch_udp *udp)
{
}

int ff_arch_udp_bind(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr)
{
}

int ff_arch_udp_read(struct ff_arch_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len)
{
}

int ff_arch_udp_write(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len)
{
}

void ff_arch_udp_disconnect(struct ff_arch_udp *udp)
{
}
