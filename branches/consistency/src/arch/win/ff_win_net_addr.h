#ifndef FF_WIN_NET_ADDR
#define FF_WIN_NET_ADDR

#include "ff_win_stdafx.h"
#include "private/arch/ff_arch_net_addr.h"

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
