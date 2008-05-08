#ifndef FF_WIN_TCP
#define FF_WIN_TCP

#include "private/arch/ff_arch_completion_port.h"

#ifdef __cplusplus
extern "C" {
#endif

void ff_win_tcp_initialize(struct ff_arch_completion_port *completion_port);

void ff_win_tcp_shutdown();

#ifdef __cplusplus
}
#endif

#endif
