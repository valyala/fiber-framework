#include "private/ff_common.h"

#include "private/arch/ff_arch_mutex.h"

#include <pthread.h>

struct ff_arch_mutex
{
	pthread_mutex_t mtx;
};

struct ff_arch_mutex *ff_arch_mutex_create()
{
	struct ff_arch_mutex *mutex;
	int rv;

	mutex = (struct ff_arch_mutex *) ff_malloc(sizeof(*mutex));
	rv = pthread_mutex_init(&mutex->mtx, NULL);
	ff_assert(rv == 0);

	return mutex;
}

void ff_arch_mutex_delete(struct ff_arch_mutex *mutex)
{
	int rv;

	rv = pthread_mutex_destroy(&mutex->mtx);
	ff_assert(rv == 0);
	ff_free(mutex);
}

void ff_arch_mutex_lock(struct ff_arch_mutex *mutex)
{
	int rv;

	rv = pthread_mutex_lock(&mutex->mtx);
	ff_assert(rv == 0);
}

void ff_arch_mutex_unlock(struct ff_arch_mutex *mutex)
{
	int rv;

	rv = pthread_mutex_unlock(&mutex->mtx);
	ff_assert(rv == 0);
}
