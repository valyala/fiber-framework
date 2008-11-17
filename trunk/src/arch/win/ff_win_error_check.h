#ifndef FF_WIN_ERROR_CHECK_H
#define FF_WIN_ERROR_CHECK_H

#include "ff_win_stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ff_winapi_fatal_error_check(expression, format, ...) \
	do { \
		if (!(expression)) \
		{ \
			DWORD last_error = GetLastError(); \
			ff_log_fatal_error(L"WINAPI fatal error at %hs:%d, GetLastError()=%lu. " format, __FILE__, __LINE__, last_error, ## __VA_ARGS__); \
		} \
	} while (0)

#define ff_winsock_fatal_error_check(expression, format, ...) \
	do { \
		if (!(expression)) \
		{ \
			int last_error = WSAGetLastError(); \
			ff_log_fatal_error(L"WinSock fatal error at %hs:%d, WSAGetLastError()=%lu. " format, __FILE__, __LINE__, last_error, ## __VA_ARGS__); \
		} \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif
