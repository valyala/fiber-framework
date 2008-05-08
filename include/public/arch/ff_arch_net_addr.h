#ifndef FF_ARCH_NET_ADDR_PUBLIC
#define FF_ARCH_NET_ADDR_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_net_addr;

struct ff_arch_net_addr *ff_arch_net_addr_create();

void ff_arch_net_addr_delete(struct ff_arch_net_addr *addr);

int ff_arch_net_addr_resolve(struct ff_arch_net_addr *addr, const wchar_t *host, int port);

#ifdef __cplusplus
}
#endif

#endif
