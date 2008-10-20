#ifndef FF_WIN_FILE
#define FF_WIN_FILE

#include "private/arch/ff_arch_completion_port.h"

#ifdef __cplusplus
extern "C" {
#endif

void ff_win_file_initialize(struct ff_arch_completion_port *completion_port);

void ff_win_file_shutdown();

#ifdef __cplusplus
}
#endif

#endif
