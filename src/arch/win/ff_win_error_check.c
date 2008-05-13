#include "ff_win_stdafx.h"

#include "ff_win_error_check.h"

#define ERROR_MESSAGE_MAX_SIZE 4096

void fatal_error(wchar_t *format, ...)
{
	wchar_t error_message[ERROR_MESSAGE_MAX_SIZE];
	int len;

	va_list args_ptr;
	va_start(args_ptr, format);
	len = vswprintf_s(error_message, ERROR_MESSAGE_MAX_SIZE, format, args_ptr);
	ff_assert(len >= 0);
	va_end(args_ptr);
	FatalAppExit(0, error_message);

	/* this is a guard, which will exit the application in the case if user
	 * choose to contiune execution of the application
	 */
	exit(EXIT_FAILURE);
}
