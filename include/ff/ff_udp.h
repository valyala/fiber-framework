#ifndef FF_UDP_PUBLIC
#define FF_UDP_PUBLIC

#include "ff/ff_common.h"
#include "ff/arch/ff_arch_net_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_udp;

enum ff_udp_type
{
	FF_UDP_BROADCAST,
	FF_UDP_UNICAST
};

FF_API struct ff_udp *ff_udp_create(enum ff_udp_type type);

FF_API void ff_udp_delete(struct ff_udp *udp);

FF_API enum ff_result ff_udp_bind(struct ff_udp *udp, const struct ff_arch_net_addr *addr);

FF_API int ff_udp_read(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len);

FF_API int ff_udp_read_with_timeout(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len, int timeout);

FF_API int ff_udp_write(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len);

FF_API int ff_udp_write_with_timeout(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len, int timeout);

FF_API void ff_udp_disconnect(struct ff_udp *udp);

#ifdef __cplusplus
}
#endif

#endif
