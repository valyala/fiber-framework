#ifndef FF_MUTEX_PUBLIC
#define FF_MUTEX_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

struct ff_mutex;

struct ff_mutex *ff_mutex_create();

void ff_mutex_delete(struct ff_mutex *mutex);

void ff_mutex_lock(struct ff_mutex *mutex);

void ff_mutex_unlock(struct ff_mutex *mutex);

#ifdef __cplusplus
}
#endif

#endif
