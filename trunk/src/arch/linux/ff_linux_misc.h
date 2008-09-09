#ifndef FF_LINUX_MISC
#define FF_LINUX_MISC

#include "private/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

char *ff_linux_misc_wide_to_multibyte_string(const wchar_t *wide_str);

#ifdef __cplusplus
}
#endif

#endif
