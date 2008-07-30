#ifndef FF_LINUX_NET
#define FF_LINUX_NET

#include "private/arch/ff_arch_completion_port.h"

#ifdef __cplusplus
extern "C" {
#endif

void ff_linux_net_initialize(struct ff_arch_completion_port *completion_port);

void ff_linux_net_shutdown();

#ifdef __cplusplus
}
#endif

#endif
