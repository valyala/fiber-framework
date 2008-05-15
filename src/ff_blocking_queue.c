#include "private/ff_common.h"

#include "private/ff_blocking_queue.h"
#include "private/ff_queue.h"
#include "private/ff_semaphore.h"

struct ff_blocking_queue
{
	struct ff_queue *simple_queue;
	struct ff_semaphore *producer_semaphore;
	struct ff_semaphore *consumer_semaphore;
};

struct ff_blocking_queue *ff_blocking_queue_create(int max_size)
{
	struct ff_blocking_queue *queue;

	ff_assert(max_size > 0);

	queue = (struct ff_blocking_queue *) ff_malloc(sizeof(*queue));
	queue->simple_queue = ff_queue_create();
	queue->producer_semaphore = ff_semaphore_create(0);
	queue->consumer_semaphore = ff_semaphore_create(max_size);

	return queue;
}

void ff_blocking_queue_delete(struct ff_blocking_queue *queue)
{
	ff_semaphore_delete(queue->consumer_semaphore);
	ff_semaphore_delete(queue->producer_semaphore);
	ff_queue_delete(queue->simple_queue);
	ff_free(queue);
}

void *ff_blocking_queue_get(struct ff_blocking_queue *queue)
{
	void *data;

	ff_semaphore_down(queue->producer_semaphore);
	data = ff_queue_front(queue->simple_queue);
	ff_queue_pop(queue->simple_queue);
	ff_semaphore_up(queue->consumer_semaphore);

	return data;
}

int ff_blocking_queue_get_with_timeout(struct ff_blocking_queue *queue, int timeout, void **data)
{
	int is_success;
	
	is_success = ff_semaphore_down_with_timeout(queue->producer_semaphore, timeout);
	if (is_success)
	{
		*data = ff_queue_front(queue->simple_queue);
		ff_queue_pop(queue->simple_queue);
		ff_semaphore_up(queue->consumer_semaphore);
	}
	return is_success;
}

void ff_blocking_queue_put(struct ff_blocking_queue *queue, void *data)
{
	ff_semaphore_down(queue->consumer_semaphore);
	ff_queue_push(queue->simple_queue, data);
	ff_semaphore_up(queue->producer_semaphore);
}
