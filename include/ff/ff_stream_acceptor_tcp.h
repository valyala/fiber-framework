#ifndef FF_STREAM_ACCEPTOR_TCP_PUBLIC
#define FF_STREAM_ACCEPTOR_TCP_PUBLIC

#include "ff/ff_common.h"
#include "ff/ff_stream_acceptor.h"
#include "ff/arch/ff_arch_net_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates tcp stream acceptor, which will accept tcp connections on the given addr.
 * This function acquires the addr, so the caller mustn't delete the addr!
 * Always returns correct result.
 */
FF_API struct ff_stream_acceptor *ff_stream_acceptor_tcp_create(struct ff_arch_net_addr *addr);

#ifdef __cplusplus
}
#endif

#endif
