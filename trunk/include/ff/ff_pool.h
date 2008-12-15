#ifndef FF_POOL_PUBLIC_H
#define FF_POOL_PUBLIC_H

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_pool;

/**
 * This callback must create valid entry for the pool.
 * entry_constructor_ctx is the value, which was passed to the ff_pool_create().
 */
typedef void *(*ff_pool_entry_constructor)(void *entry_constructor_ctx);

/**
 * This callback must delete the given entry.
 * entry_destructor_ctx is the value, which was passed to the ff_pool_create().
 */
typedef void (*ff_pool_entry_destructor)(void *entry_destructor_ctx, void *entry);

/**
 * Creates a pool of entries with maximum size of max_size. Entries are created using the entry_constructor and deleted using the entry_destructor.
 * entry_constructor_ctx is passed to the entry_constructor and entry_destructor_ctx is passed to the entry_destructor.
 * Always returns correct result.
 */
FF_API struct ff_pool *ff_pool_create(int max_size, ff_pool_entry_constructor entry_constructor, void *entry_constructor_ctx, ff_pool_entry_destructor entry_destructor, void *entry_destructor_ctx);

/**
 * Deletes the pool and all entries in it.
 */
FF_API void ff_pool_delete(struct ff_pool *pool);

/**
 * Acquires an entry from the pool.
 * This function can block if there are no entries at the pool at the moment.
 * It will wait while another fiber will call ff_pool_release_entry().
 */
FF_API void ff_pool_acquire_entry(struct ff_pool *pool, void **entry);

/**
 * Acquires an entry from the pool.
 * If the pool is empty this function waits for the entry during the given timeout.
 * Returns FF_SUCCESS if the entry was successfully acquired, FF_FAILURE if an entry
 * couldn't be acquired during the given timeout.
 */
FF_API enum ff_result ff_pool_acquire_entry_with_timeout(struct ff_pool *pool, void **entry, int timeout);

/**
 * Releases the given entry to the pool.
 * It is expected that the entry has been acquired using ff_pool_acquire_entry() before.
 */
FF_API void ff_pool_release_entry(struct ff_pool *pool, void *entry);

#ifdef __cplusplus
}
#endif

#endif
