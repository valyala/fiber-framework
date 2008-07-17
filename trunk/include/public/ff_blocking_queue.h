#ifndef FF_BLOCKING_QUEUE_PUBLIC
#define FF_BLOCKING_QUEUE_PUBLIC

#include "public/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_blocking_queue;

FF_API struct ff_blocking_queue *ff_blocking_queue_create(int max_size);

FF_API void ff_blocking_queue_delete(struct ff_blocking_queue *queue);

FF_API void ff_blocking_queue_get(struct ff_blocking_queue *queue, void **data);

FF_API int ff_blocking_queue_get_with_timeout(struct ff_blocking_queue *queue, void **data, int timeout);

FF_API void ff_blocking_queue_put(struct ff_blocking_queue *queue, void *data);

FF_API int ff_blocking_queue_put_with_timeout(struct ff_blocking_queue *queue, void *data, int timeout);

#ifdef __cplusplus
}
#endif

#endif
