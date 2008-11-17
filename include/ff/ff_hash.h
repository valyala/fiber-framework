#ifndef FF_HASH_PUBLIC_H
#define FF_HASH_PUBLIC_H

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * calculates hash value for the 32bit buffer with the given buf_size size and the given start_value.
 */
FF_API uint32_t ff_hash_uint32(uint32_t start_value, const uint32_t *buf, int buf_size);

/**
 * calculates hash value for the 16bit buffer with the given buf_size size and the given start_value.
 * This function is slower than the ff_hash_uint32() on little-endian architectures.
 */
FF_API uint32_t ff_hash_uint16(uint32_t start_value, const uint16_t *buf, int buf_size);

/**
 * calculates hash vlaue for the 8bit buffer with the given bif_size size and the given start_value.
 */
FF_API uint32_t ff_hash_uint8(uint32_t start_value, const uint8_t *buf, int buf_size);

#ifdef __cplusplus
}
#endif

#endif
