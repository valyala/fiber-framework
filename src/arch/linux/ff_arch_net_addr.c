#include "private/arch/ff_arch_net_addr.h"
#include "ff_linux_net_addr.h"

#include <string.h>

struct ff_arch_net_addr *ff_arch_net_addr_create()
{
	struct ff_arch_net_addr *addr;

	addr = (struct ff_arch_net_addr *) ff_malloc(sizeof(*addr));

	return addr;
}

void ff_arch_net_addr_delete(struct ff_arch_net_addr *addr)
{
	ff_free(addr);
}

int ff_arch_net_addr_resolve(struct ff_arch_net_addr *addr, const wchar_t *host, int port)
{
}

void ff_arch_net_addr_get_broadcast_addr(const struct ff_arch_net_addr *addr, const struct ff_arch_net_addr *net_mask, struct ff_arch_net_addr *broadcast_addr)
{
	struct sockaddr_in tmp_addr;

	memcpy(&tmp_addr, &addr->addr, sizeof(tmp_addr));
	tmp_addr.sin_addr.s_addr = addr->addr.sin_addr.s_addr | ~(net_mask->addr.sin_addr.s_addr);
	memcpy(&broadcast_addr->addr, &tmp_addr, sizeof(tmp_addr));
}

int ff_arch_net_addr_is_equal(const struct ff_arch_net_addr *addr1, const struct ff_arch_net_addr *addr2)
{
	int is_equal;
	
	is_equal = (memcmp(&addr1->addr, &addr2->addr, sizeof(addr1->addr)) == 0);
	return is_equal;
}
