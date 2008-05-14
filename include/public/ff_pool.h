#ifndef FF_POOL_PUBLIC
#define FF_POOL_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

struct ff_pool;

typedef void *(*ff_pool_entry_constructor)(void *ctx);

typedef void (*ff_pool_entry_destructor)(void *entry);

struct ff_pool *ff_pool_create(int max_size, ff_pool_entry_constructor entry_constructor, void *entry_constructor_ctx, ff_pool_entry_destructor entry_destructor);

void ff_pool_delete(struct ff_pool *pool);

void *ff_pool_acquire_entry(struct ff_pool *pool);

void ff_pool_release_entry(struct ff_pool *pool, void *entry);

#ifdef __cplusplus
}
#endif

#endif
