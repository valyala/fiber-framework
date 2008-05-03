#ifndef FF_ARCH_LOCK_PRIVATE
#define FF_ARCH_LOCK_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_lock;

struct ff_arch_lock *ff_arch_lock_create();

void ff_arch_lock_delete(struct ff_arch_lock *lock);

void ff_arch_lock_lock(struct ff_arch_lock *lock);

void ff_arch_lock_unlock(struct ff_arch_lock *lock);

#ifdef __cplusplus
}
#endif

#endif
