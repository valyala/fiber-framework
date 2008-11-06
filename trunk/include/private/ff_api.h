#ifndef FF_API_PRIVATE
#define FF_API_PRIVATE

#include "ff/ff_api.h"

#if !defined(FF_API)
	#error ff/ff_api.h file must define FF_API
#endif

/* undefine public definition of the FF_API */
#undef FF_API

#if defined(WIN32)
	#define FF_API __declspec(dllexport)
#else
	#define FF_API __attribute__((visibility("default")))
#endif

#endif
