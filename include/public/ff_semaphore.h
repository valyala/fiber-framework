#ifndef FF_SEMAPHORE_PUBLIC
#define FF_SEMAPHORE_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

struct ff_semaphore;

struct ff_semaphore *ff_semaphore_create(int value);

void ff_semaphore_delete(struct ff_semaphore *semaphore);

void ff_semaphore_up(struct ff_semaphore *semaphore);

void ff_semaphore_down(struct ff_semaphore *semaphore);

#ifdef __cplusplus
}
#endif

#endif
