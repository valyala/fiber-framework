#ifndef FF_LINUX_NET_ADDR
#define FF_LINUX_NET_ADDR

#include "private/arch/ff_arch_net_addr.h"

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_net_addr
{
	struct sockaddr_in addr;
};

#ifdef __cplusplus
}
#endif

#endif
