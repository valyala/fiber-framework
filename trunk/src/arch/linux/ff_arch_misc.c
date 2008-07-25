#include "private/arch/ff_arch_misc.h"
#include "private/arch/ff_arch_completion_port.h"

void ff_arch_misc_initialize(struct ff_arch_completion_port *completion_port)
{
	ff_linux_net_initialize(completion_port);
	ff_linux_file_initialize(completion_port);
}

void ff_arch_misc_shutdown()
{
	ff_linux_file_shutdown();
	ff_linux_net_shutdown();
}

int64_t ff_arch_misc_get_current_time()
{
}

void ff_arch_misc_sleep(int interval)
{
}

int ff_arch_misc_get_cpus_cnt()
{
}
