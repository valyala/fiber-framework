#ifndef FF_API_PUBLIC
#define FF_API_PUBLIC

#if defined(FF_BUILD_DLL)
	#define FF_API __declspec(dllexport)
#elif defined(FF_USE_DLL)
	#define FF_API __declspec(dllimport)
#else
	#define FF_API __attribute__((visibility("default")))
#endif

#endif
