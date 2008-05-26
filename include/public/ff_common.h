#ifndef FF_COMMON_PUBLIC
#define FF_COMMON_PUBLIC

#if defined(FF_BUILD_DLL)
	#define FF_API __declspec(dllexport)
#elif defined(FF_USE_DLL)
	#define FF_API __declspec(dllimport)
#else
	#define FF_API extern
#endif

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

#endif
