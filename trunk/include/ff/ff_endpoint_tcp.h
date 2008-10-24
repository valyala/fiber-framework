#ifndef FF_ENDPOINT_TCP_PUBLIC
#define FF_ENDPOINT_TCP_PUBLIC

#include "ff/ff_common.h"
#include "ff/ff_endpoint.h"
#include "ff/arch/ff_arch_net_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

FF_API struct ff_endpoint *ff_endpoint_tcp_create(struct ff_arch_net_addr *addr);

#ifdef __cplusplus
}
#endif

#endif
