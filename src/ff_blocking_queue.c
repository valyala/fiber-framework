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

void ff_blocking_queue_get(struct ff_blocking_queue *queue, const void **data)
{
	ff_semaphore_down(queue->producer_semaphore);
	ff_queue_front(queue->simple_queue, data);
	ff_queue_pop(queue->simple_queue);
	ff_semaphore_up(queue->consumer_semaphore);
}

enum ff_result ff_blocking_queue_get_with_timeout(struct ff_blocking_queue *queue, const void **data, int timeout)
{
	enum ff_result result;

	ff_assert(timeout > 0);
	
	result = ff_semaphore_down_with_timeout(queue->producer_semaphore, timeout);
	if (result == FF_SUCCESS)
	{
		ff_queue_front(queue->simple_queue, data);
		ff_queue_pop(queue->simple_queue);
		ff_semaphore_up(queue->consumer_semaphore);
	}
	else
	{
		ff_log_debug(L"cannot get data from the blocking queue=%p during the timeout=%d", queue, timeout);
	}
	return result;
}

void ff_blocking_queue_put(struct ff_blocking_queue *queue, const void *data)
{
	ff_semaphore_down(queue->consumer_semaphore);
	ff_queue_push(queue->simple_queue, data);
	ff_semaphore_up(queue->producer_semaphore);
}

enum ff_result ff_blocking_queue_put_with_timeout(struct ff_blocking_queue *queue, const void *data, int timeout)
{
	enum ff_result result;

	ff_assert(timeout > 0);

	result = ff_semaphore_down_with_timeout(queue->consumer_semaphore, timeout);
	if (result == FF_SUCCESS)
	{
		ff_queue_push(queue->simple_queue, data);
		ff_semaphore_up(queue->producer_semaphore);
	}
	else
	{
		ff_log_debug(L"cannot put data=%p to the blocking queue=%p during the timeout=%d", data, queue, timeout);
	}
	return result;
}

int ff_blocking_queue_is_empty(struct ff_blocking_queue *queue)
{
	int is_empty;

	is_empty = ff_queue_is_empty(queue->simple_queue);
	return is_empty;
}
