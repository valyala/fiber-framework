#ifndef FF_ARCH_UDP_PRIVATE
#define FF_ARCH_UDP_PRIVATE

#include "private/arch/ff_arch_net_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_udp;

struct ff_arch_udp *ff_arch_udp_create(int is_broadcast);

void ff_arch_udp_delete(struct ff_arch_udp *udp);

enum ff_result ff_arch_udp_bind(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr);

int ff_arch_udp_read(struct ff_arch_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len);

int ff_arch_udp_write(struct ff_arch_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len);

void ff_arch_udp_disconnect(struct ff_arch_udp *udp);

#ifdef __cplusplus
}
#endif

#endif
