#ifndef FF_POOL_PUBLIC
#define FF_POOL_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_pool;

typedef void *(*ff_pool_entry_constructor)(void *ctx);

typedef void (*ff_pool_entry_destructor)(void *entry);

FF_API struct ff_pool *ff_pool_create(int max_size, ff_pool_entry_constructor entry_constructor, void *entry_constructor_ctx, ff_pool_entry_destructor entry_destructor);

FF_API void ff_pool_delete(struct ff_pool *pool);

FF_API void *ff_pool_acquire_entry(struct ff_pool *pool);

FF_API void ff_pool_release_entry(struct ff_pool *pool, void *entry);

typedef void (*ff_pool_visitor_func)(void *entry, void *ctx, int is_acquired);

FF_API void ff_pool_for_each_entry(struct ff_pool *pool, ff_pool_visitor_func visitor_func, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
