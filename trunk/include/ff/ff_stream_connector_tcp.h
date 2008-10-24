#ifndef FF_STREAM_CONNECTOR_TCP_PUBLIC
#define FF_STREAM_CONNECTOR_TCP_PUBLIC

#include "ff/ff_common.h"
#include "ff/ff_stream_connector.h"
#include "ff/arch/ff_arch_net_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

FF_API struct ff_stream_connector *ff_stream_connector_tcp_create(struct ff_arch_net_addr *addr);

#ifdef __cplusplus
}
#endif

#endif
