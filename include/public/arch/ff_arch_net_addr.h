#ifndef FF_ARCH_NET_ADDR_PUBLIC
#define FF_ARCH_NET_ADDR_PUBLIC

#include "public/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_net_addr;

FF_API struct ff_arch_net_addr *ff_arch_net_addr_create();

FF_API void ff_arch_net_addr_delete(struct ff_arch_net_addr *addr);

FF_API int ff_arch_net_addr_resolve(struct ff_arch_net_addr *addr, const wchar_t *host, int port);

FF_API void ff_arch_net_addr_get_broadcast_addr(const struct ff_arch_net_addr *addr, const struct ff_arch_net_addr *net_mask, struct ff_arch_net_addr *broadcast_addr);

FF_API int ff_arch_net_addr_is_equal(const struct ff_arch_net_addr *addr1, const struct ff_arch_net_addr *addr2);

#ifdef __cplusplus
}
#endif

#endif
