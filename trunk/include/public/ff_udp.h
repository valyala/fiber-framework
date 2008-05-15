#ifndef FF_UDP_PUBLIC
#define FF_UDP_PUBLIC

#include "public/arch/ff_arch_net_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_udp;

struct ff_udp *ff_udp_create(int is_broadcast);

void ff_udp_delete(struct ff_udp *udp);

int ff_udp_bind(struct ff_udp *udp, const struct ff_arch_net_addr *addr);

int ff_udp_read(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len);

int ff_udp_read_with_timeout(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len, int timeout);

int ff_udp_write(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len);

int ff_udp_write_with_timeout(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len, int timeout);

void ff_udp_disconnect(struct ff_udp *udp);

#ifdef __cplusplus
}
#endif

#endif
