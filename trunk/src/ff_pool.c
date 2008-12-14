#include "private/ff_common.h"

#include "private/ff_pool.h"
#include "private/ff_semaphore.h"
#include "private/ff_stack.h"

struct ff_pool
{
	ff_pool_entry_constructor entry_constructor;
	ff_pool_entry_destructor entry_destructor;
	void *entry_constructor_ctx;
	struct ff_semaphore *semaphore;
	struct ff_stack *free_entries;
	int max_size;
	int current_size;
	int busy_entries_cnt;
};

static void get_free_entry(struct ff_pool *pool, void **entry)
{
        struct ff_stack *free_entries;

        ff_assert(pool != NULL);
        ff_assert(entry != NULL);

        ff_assert(pool->current_size <= pool->max_size);
        ff_assert(pool->busy_entries_cnt <= pool->current_size);
        ff_assert(pool->busy_entries_cnt < pool->max_size);
        ff_assert(pool->busy_entries_cnt >= 0);
        free_entries = pool->free_entries;
        if (pool->busy_entries_cnt == pool->current_size)
        {
                void *new_entry;

                new_entry = pool->entry_constructor(pool->entry_constructor_ctx);
                ff_stack_push(free_entries, new_entry);
                pool->current_size++;
        }
        ff_stack_top(free_entries, (const void **) entry);
        ff_stack_pop(free_entries);
        pool->busy_entries_cnt++;
}

struct ff_pool *ff_pool_create(int max_size, ff_pool_entry_constructor entry_constructor, void *entry_constructor_ctx, ff_pool_entry_destructor entry_destructor)
{
	struct ff_pool *pool;

	ff_assert(max_size > 0);
	ff_assert(entry_constructor != NULL);
	ff_assert(entry_destructor != NULL);

	pool = (struct ff_pool *) ff_malloc(sizeof(*pool));
	pool->entry_constructor = entry_constructor;
	pool->entry_destructor = entry_destructor;
	pool->entry_constructor_ctx = entry_constructor_ctx;
	pool->semaphore = ff_semaphore_create(max_size);
	pool->free_entries = ff_stack_create();
	pool->max_size = max_size;
	pool->current_size = 0;
	pool->busy_entries_cnt = 0;

	return pool;
}

void ff_pool_delete(struct ff_pool *pool)
{
	struct ff_stack *free_entries;
	int i;
	int current_size;

	ff_assert(pool->busy_entries_cnt == 0);
	free_entries = pool->free_entries;
	current_size = pool->current_size;
	for (i = 0; i < current_size; i++)
	{
		void *entry;

		ff_stack_top(free_entries, (const void **) &entry);
		ff_stack_pop(free_entries);
		pool->entry_destructor(entry);
	}
	ff_stack_delete(free_entries);
	ff_semaphore_delete(pool->semaphore);
	ff_free(pool);
}

void ff_pool_acquire_entry(struct ff_pool *pool, void **entry)
{
	ff_assert(pool != NULL);
	ff_assert(entry != NULL);

	ff_semaphore_down(pool->semaphore);
	get_free_entry(pool, entry);
}

enum ff_result ff_pool_acquire_entry_with_timeout(struct ff_pool *pool, void **entry, int timeout)
{
	enum ff_result result;

	ff_assert(pool != NULL);
	ff_assert(entry != NULL);
	ff_assert(timeout > 0);

	result = ff_semaphore_down_with_timeout(pool->semaphore, timeout);
	if (result == FF_SUCCESS)
	{
		get_free_entry(pool, entry);
	}
	else
	{
		ff_log_debug(L"cannot acquire entry from the pool=%p during the timeout=%d", pool, timeout);
	}

	return result;
}

void ff_pool_release_entry(struct ff_pool *pool, void *entry)
{
	ff_assert(pool->current_size <= pool->max_size);
	ff_assert(pool->busy_entries_cnt <= pool->current_size);
	ff_assert(pool->busy_entries_cnt > 0);
	ff_stack_push(pool->free_entries, entry);
	pool->busy_entries_cnt--;

	ff_semaphore_up(pool->semaphore);
}

