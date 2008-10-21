#ifndef FF_ARCH_NET_ADDR_PUBLIC
#define FF_ARCH_NET_ADDR_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_net_addr;

FF_API struct ff_arch_net_addr *ff_arch_net_addr_create();

FF_API void ff_arch_net_addr_delete(struct ff_arch_net_addr *addr);

FF_API enum ff_result ff_arch_net_addr_resolve(struct ff_arch_net_addr *addr, const wchar_t *host, int port);

FF_API void ff_arch_net_addr_get_broadcast_addr(const struct ff_arch_net_addr *addr, const struct ff_arch_net_addr *net_mask, struct ff_arch_net_addr *broadcast_addr);

FF_API int ff_arch_net_addr_is_equal(const struct ff_arch_net_addr *addr1, const struct ff_arch_net_addr *addr2);

FF_API const wchar_t *ff_arch_net_addr_to_string(const struct ff_arch_net_addr *addr);

FF_API void ff_arch_net_addr_delete_string(const wchar_t *str);

#ifdef __cplusplus
}
#endif

#endif
