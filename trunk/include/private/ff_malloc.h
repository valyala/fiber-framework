#ifndef FF_MALLOC_PRIVATE
#define FF_MALLOC_PRIVATE

#include <stdlib.h> /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * allocates size bytes of memory.
 * Always returns correct result.
 */
void *ff_malloc(size_t size);

/**
 * @public
 * frees the memory, which was allocated by ff_malloc()
 */
void ff_free(void *mem);

#ifdef __cplusplus
}
#endif

#endif
