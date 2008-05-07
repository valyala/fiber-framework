#ifndef FF_ARCH_MUTEX_PRIVATE
#define FF_ARCH_MUTEX_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_mutex;

struct ff_arch_mutex *ff_arch_mutex_create();

void ff_arch_mutex_delete(struct ff_arch_mutex *mutex);

void ff_arch_mutex_lock(struct ff_arch_mutex *mutex);

void ff_arch_mutex_unlock(struct ff_arch_mutex *mutex);

#ifdef __cplusplus
}
#endif

#endif
