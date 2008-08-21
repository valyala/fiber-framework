#include "private/ff_common.h"

#include "ff_linux_net.h"
#include "private/ff_core.h"
#include "private/ff_fiber.h"
#include "private/arch/ff_arch_completion_port.h"
#include "ff_linux_completion_port.h"

struct net_data
{
	struct ff_arch_completion_port *completion_port;
};

static struct net_data net_ctx;

void ff_linux_net_initialize(struct ff_arch_completion_port *completion_port)
{
	net_ctx.completion_port = completion_port;
}

void ff_linux_net_shutdown()
{
	/* nothing to do */
}

void ff_linux_net_wait_for_io(int sd, enum ff_linux_net_io_type io_type)
{
	struct ff_fiber *current_fiber;
	enum ff_linux_completion_port_operation_type operation_type;

	current_fiber = ff_fiber_get_current();
	operation_type = (io_type == FF_LINUX_NET_IO_READ) ? FF_COMPLETION_PORT_OPERATION_READ : FF_COMPLETION_PORT_OPERATION_WRITE;
	ff_linux_completion_port_register_operation(net_ctx.completion_port, sd, operation_type, current_fiber);
	ff_core_yield_fiber();
}

void ff_linux_net_wakeup_fiber(struct ff_fiber *fiber)
{
	ff_arch_completion_port_put(net_ctx.completion_port, fiber);
}
