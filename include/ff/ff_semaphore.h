#ifndef FF_SEMAPHORE_PUBLIC_H
#define FF_SEMAPHORE_PUBLIC_H

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_semaphore;

FF_API struct ff_semaphore *ff_semaphore_create(int value);

FF_API void ff_semaphore_delete(struct ff_semaphore *semaphore);

FF_API void ff_semaphore_up(struct ff_semaphore *semaphore);

FF_API void ff_semaphore_down(struct ff_semaphore *semaphore);

FF_API enum ff_result ff_semaphore_down_with_timeout(struct ff_semaphore *semaphore, int timeout);

#ifdef __cplusplus
}
#endif

#endif
