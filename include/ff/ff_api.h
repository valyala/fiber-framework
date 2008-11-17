#ifndef FF_API_PUBLIC_H
#define FF_API_PUBLIC_H

#if defined(WIN32)
	#define FF_API __declspec(dllimport)
#else
	#define FF_API __attribute__((externally_visible))
#endif

#endif
