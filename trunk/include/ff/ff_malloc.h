#ifndef FF_MALLOC_PUBLIC_H
#define FF_MALLOC_PUBLIC_H

#include <stdlib.h> /* for size_t */
#include "ff/ff_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * allocates size bytes of memory.
 * Always returns correct result.
 */
FF_API void *ff_malloc(size_t size);

/**
 * @public
 * allocates memory for an array of nmemb elements of size bytes each
 * and returns a pointer to the allocated memory.
 * The memory is set to zero.
 * Always returns correct result.
 */
FF_API void *ff_calloc(size_t nmemb, size_t size);

/**
 * @public
 * frees the memory, which was allocated by ff_malloc() or ff_calloc()
 */
FF_API void ff_free(void *mem);

#ifdef __cplusplus
}
#endif

#endif
