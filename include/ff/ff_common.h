#ifndef FF_COMMON_PUBLIC
#define FF_COMMON_PUBLIC

#include <wchar.h>

#ifdef HAS_STDINT_H
#include <stdint.h>
#else
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int8 int8_t;

typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
#endif

enum ff_result
{
	FF_SUCCESS,
	FF_FAILURE
};

#include "ff/ff_api.h"
#include "ff/ff_malloc.h"
#include "ff/ff_assert.h"
#include "ff/ff_log.h"

#endif
