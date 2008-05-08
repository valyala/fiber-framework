#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_misc.h"

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
