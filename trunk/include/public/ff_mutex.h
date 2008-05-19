#ifndef FF_MUTEX_PUBLIC
#define FF_MUTEX_PUBLIC

#include "public/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_mutex;

FF_API struct ff_mutex *ff_mutex_create();

FF_API void ff_mutex_delete(struct ff_mutex *mutex);

FF_API void ff_mutex_lock(struct ff_mutex *mutex);

FF_API void ff_mutex_unlock(struct ff_mutex *mutex);

#ifdef __cplusplus
}
#endif

#endif
