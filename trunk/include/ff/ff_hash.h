#ifndef FF_HASH_PUBLIC
#define FF_HASH_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ff_hash(uint32_t start_value, const uint32_t *buf, int buf_size);

#ifdef __cplusplus
}
#endif

#endif
