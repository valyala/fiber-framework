#ifndef FF_COMMON_PRIVATE
#define FF_COMMON_PRIVATE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ff/ff_common.h"

#include "private/ff_malloc.h"
#include "private/ff_assert.h"

#ifdef HAS_THREAD_KEYWORD
	#define FF_PER_THREAD __thread
#else
	#define FF_PER_THREAD __declspec(thread)
#endif

#endif
