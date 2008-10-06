#ifndef FF_HASH_PUBLIC
#define FF_HASH_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

FF_API uint32_t ff_hash_uint32(uint32_t start_value, const uint32_t *buf, int buf_size);

FF_API uint32_t ff_hash_uint16(uint32_t start_value, const uint16_t *buf, int buf_size);

FF_API uint32_t ff_hash_uint8(uint32_t start_value, const uint8_t *buf, int buf_size);

#ifdef __cplusplus
}
#endif

#endif
