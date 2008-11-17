#ifndef FF_LINUX_COMPLETION_PORT_H
#define FF_LINUX_COMPLETION_PORT_H

#include "private/arch/ff_arch_completion_port.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ff_linux_completion_port_operation_type
{
	FF_COMPLETION_PORT_OPERATION_READ,
	FF_COMPLETION_PORT_OPERATION_WRITE
};

void ff_linux_completion_port_register_operation(struct ff_arch_completion_port *completion_port, int fd, enum ff_linux_completion_port_operation_type operation_type, const void *data);

#ifdef __cplusplus
}
#endif

#endif
