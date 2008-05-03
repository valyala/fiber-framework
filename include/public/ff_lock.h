#ifndef FF_LOCK_PUBLIC
#define FF_LOCK_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

struct ff_lock;

struct ff_lock *ff_lock_create();

void ff_lock_delete(struct ff_lock *lock);

void ff_lock_lock(struct ff_lock *lock);

void ff_lock_unlock(struct ff_lock *lock);

#ifdef __cplusplus
}
#endif

#endif
