#ifndef FF_QUEUE_PRIVATE
#define FF_QUEUE_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_queue;

struct ff_queue *ff_queue_create();

void ff_queue_delete(struct ff_queue *queue);

void ff_queue_push(struct ff_queue *queue, const void *data);

void ff_queue_front(struct ff_queue *queue, const void **data);

void ff_queue_pop(struct ff_queue *queue);

int ff_queue_is_empty(struct ff_queue *queue);

#ifdef __cplusplus
}
#endif

#endif
