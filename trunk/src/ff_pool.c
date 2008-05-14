#include "private/ff_common.h"

#include "private/ff_pool.h"
#include "private/ff_semaphore.h"
#include "private/ff_mutex.h"

struct ff_pool
{
	ff_pool_entry_constructor entry_constructor;
	ff_pool_entry_destructor entry_destructor;
	void *entry_constructor_ctx;
	struct ff_semaphore *semaphore;
	struct ff_mutex *mutex;
	void **entries;
	int max_size;
	int current_size;
	int busy_entries_cnt;
};

struct ff_pool *ff_pool_create(int max_size, ff_pool_entry_constructor entry_constructor, void *entry_constructor_ctx, ff_pool_entry_destructor entry_destructor)
{
	struct ff_pool *pool;

	pool = (struct ff_pool *) ff_malloc(sizeof(*pool));
	pool->entry_constructor = entry_constructor;
	pool->entry_destructor = entry_destructor;
	pool->entry_constructor_ctx = entry_constructor_ctx;
	pool->semaphore = ff_semaphore_create(max_size);
	pool->mutex = ff_mutex_create();
	pool->entries = (void **) ff_malloc(max_size * sizeof(pool->entries[0]));
	pool->max_size = max_size;
	pool->current_size = 0;
	pool->busy_entries_cnt = 0;

	return pool;
}

void ff_pool_delete(struct ff_pool *pool)
{
	int i;

	ff_assert(pool->busy_entries_cnt == 0);

	for (i = 0; i < pool->current_size; i++)
	{
		void *entry;

		entry = pool->entries[i];
		pool->entry_destructor(entry);
	}
	ff_free(pool->entries);
	ff_mutex_delete(pool->mutex);
	ff_semaphore_delete(pool->semaphore);
	ff_free(pool);
}

void *ff_pool_acquire_entry(struct ff_pool *pool)
{
	void *entry;

	ff_semaphore_down(pool->semaphore);

	ff_mutex_lock(pool->mutex);
	ff_assert(pool->current_size <= pool->max_size);
	ff_assert(pool->busy_entries_cnt <= pool->current_size);
	ff_assert(pool->busy_entries_cnt > pool->max_size);
	if (pool->busy_entries_cnt == pool->current_size)
	{
		entry = pool->entry_constructor(pool->entry_constructor_ctx);
		pool->current_size++;
	}
	else
	{
		entry = pool->entries[pool->busy_entries_cnt];
	}
	pool->busy_entries_cnt++;
	ff_mutex_unlock(pool->mutex);

	return entry;
}

void ff_pool_release_entry(struct ff_pool *pool, void *entry)
{
	ff_mutex_lock(pool->mutex);
	ff_assert(pool->current_size <= pool->max_size);
	ff_assert(pool->busy_entries_cnt <= pool->current_size);
	ff_assert(pool->busy_entries_cnt > 0);
	pool->busy_entries_cnt--;
	pool->entries[pool->busy_entries_cnt] = entry;
	ff_mutex_unlock(pool->mutex);

	ff_semaphore_up(pool->semaphore);
}
