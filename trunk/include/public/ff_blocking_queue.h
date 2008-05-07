#ifndef FF_BLOCKING_QUEUE_PUBLIC
#define FF_BLOCKING_QUEUE_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

struct ff_blocking_queue;

struct ff_blocking_queue *ff_blocking_queue_create(int max_size);

void ff_blocking_queue_delete(struct ff_blocking_queue *queue);

void *ff_blocking_queue_get(struct ff_blocking_queue *queue);

void ff_blocking_queue_put(struct ff_blocking_queue *queue, void *data);

#ifdef __cplusplus
}
#endif

#endif
