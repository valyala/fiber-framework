#ifndef FF_LINUX_NET
#define FF_LINUX_NET

#include "private/arch/ff_arch_completion_port.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ff_linux_net_io_type
{
	FF_LINUX_NET_IO_READ,
	FF_LINUX_NET_IO_WRITE
};

void ff_linux_net_initialize(struct ff_arch_completion_port *completion_port);

void ff_linux_net_shutdown();

void ff_linux_net_wait_for_io(int sd, enum ff_linux_net_io_type io_type);

#ifdef __cplusplus
}
#endif

#endif
