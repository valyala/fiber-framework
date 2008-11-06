#ifndef FF_API_PUBLIC
#define FF_API_PUBLIC

#if defined(WIN32)
	#define FF_API __declspec(dllimport)
#else
	#define FF_API
#endif

#endif
