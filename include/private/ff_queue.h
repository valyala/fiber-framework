#ifndef FF_QUEUE_PRIVATE
#define FF_QUEUE_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_queue;

struct ff_queue *ff_queue_create();

void ff_queue_delete(struct ff_queue *queue);

void ff_queue_push(struct ff_queue *queue, void *data);

int ff_queue_is_empty(struct ff_queue *queue);

void *ff_queue_front(struct ff_queue *queue);

void ff_queue_pop(struct ff_queue *queue);

int ff_queue_remove_entry(struct ff_queue *queue, void *data);

typedef void (*ff_queue_for_each_func)(void *data, void *ctx);

void ff_queue_for_each(struct ff_queue *queue, ff_queue_for_each_func for_each_func, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
