#ifndef FF_API_PRIVATE
#define FF_API_PRIVATE

#include "ff/ff_api.h"

#if !defined(FF_API)
	#error ff/ff_api.h file must define FF_API
#endif

#if defined(WIN32)
	#undef FF_API
	#define FF_API __declspec(dllexport)
#endif

#endif
