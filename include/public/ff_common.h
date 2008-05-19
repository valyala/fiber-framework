#ifndef FF_COMMON_PUBLIC
#define FF_COMMON_PUBLIC

#if defined(FF_BUILD_DLL)
	#define FF_API __declspec(dllexport)
#elif defined(FF_USE_DLL)
	#define FF_API __declspec(dllimport)
#else
	#define FF_API extern
#endif

#endif
