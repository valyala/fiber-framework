#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_misc.h"
#include "private/arch/ff_arch_completion_port.h"
#include "ff_win_net.h"
#include "ff_win_file.h"

void ff_arch_misc_initialize(struct ff_arch_completion_port *completion_port)
{
	ff_win_net_initialize(completion_port);
	ff_win_file_initialize(completion_port);
}

void ff_arch_misc_shutdown()
{
	ff_win_file_shutdown();
	ff_win_net_shutdown();
}

int64_t ff_arch_misc_get_current_time()
{
	int64_t current_time;

	current_time = GetTickCount();
	return current_time;
}

void ff_arch_misc_sleep(int interval)
{
	Sleep(interval);
}

int ff_arch_misc_get_cpus_cnt()
{
	int cpus_cnt;
	SYSTEM_INFO system_info;

	GetSystemInfo(&system_info);
	cpus_cnt = (int) system_info.dwNumberOfProcessors;
	return cpus_cnt;
}
