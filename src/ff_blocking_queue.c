#include "private/ff_common.h"

#include "private/ff_blocking_queue.h"
#include "private/ff_queue.h"
#include "private/ff_event.h"

struct ff_blocking_queue
{
	struct ff_queue *simple_queue;
	struct ff_event *empty_event;
	struct ff_event *full_event;
	int current_size;
	int max_size;
};

struct ff_blocking_queue *ff_blocking_queue_create(int max_size)
{
	struct ff_blocking_queue *queue;

	ff_assert(max_size > 0);

	queue = (struct ff_blocking_queue *) ff_malloc(sizeof(*queue));
	queue->simple_queue = ff_queue_create();
	queue->empty_event = ff_event_create(FF_EVENT_AUTO);
	queue->full_event = ff_event_create(FF_EVENT_AUTO);
	queue->current_size = 0;
	queue->max_size = max_size;

	return queue;
}

void ff_blocking_queue_delete(struct ff_blocking_queue *queue)
{
	ff_assert(queue->max_size > 0);
	ff_assert(queue->current_size == 0);

	ff_event_delete(queue->full_event);
	ff_event_delete(queue->empty_event);
	ff_queue_delete(queue->simple_queue);
	ff_free(queue);
}

void *ff_blocking_queue_get(struct ff_blocking_queue *queue)
{
	void *data;

	ff_assert(queue->max_size > 0);
	ff_assert(queue->current_size >= 0);
	ff_assert(queue->current_size <= queue->max_size);

	while (queue->current_size == 0)
	{
		ff_event_wait(queue->empty_event);
	}

	ff_assert(queue->current_size > 0);

	data = ff_queue_front(queue->simple_queue);
	ff_queue_pop(queue->simple_queue);
	queue->current_size--;

	if (queue->current_size > 0)
	{
		ff_event_set(queue->empty_event);
	}

	if (queue->current_size + 1 == queue->max_size)
	{
		ff_event_set(queue->full_event);
	}

	return data;
}

void ff_blocking_queue_put(struct ff_blocking_queue *queue, void *data)
{
	ff_assert(queue->max_size > 0);
	ff_assert(queue->current_size >= 0);
	ff_assert(queue->current_size <= queue->max_size);

	while (queue->current_size == queue->max_size)
	{
		ff_event_wait(queue->full_event);
	}

	ff_assert(queue->current_size < queue->max_size);

	ff_queue_push(queue->simple_queue, data);
	queue->current_size++;

	if (queue->current_size < queue->max_size)
	{
		ff_event_set(queue->full_event);
	}

	if (queue->current_size == 1)
	{
		ff_event_set(queue->empty_event);
	}
}
