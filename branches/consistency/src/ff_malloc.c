#include "private/ff_common.h"

#include "private/ff_malloc.h"
#include <stdlib.h>

static void check_mem(void *mem)
{
	if (mem == NULL)
	{
		*(int *)0 = 0;
	}
}

void *ff_malloc(size_t size)
{
	void *mem = malloc(size);
	check_mem(mem);
	return mem;
}

void *ff_calloc(size_t nmemb, size_t size)
{
	void *mem = calloc(nmemb, size);
	check_mem(mem);
	return mem;
}

void ff_free(void *mem)
{
	check_mem(mem);
	free(mem);
}
