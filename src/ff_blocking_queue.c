#include "private/ff_common.h"

#include "private/ff_blocking_queue.h"
#include "private/ff_queue.h"
#include "private/ff_event.h"

struct ff_blocking_queue
{
	struct ff_queue *simple_queue;
	struct ff_event *event;
};

struct ff_blocking_queue *ff_blocking_queue_create()
{
	struct ff_blocking_queue *queue = (struct ff_blocking_queue *) ff_malloc(sizeof(*queue));
	queue->simple_queue = ff_queue_create();
	queue->event = ff_event_create(FF_EVENT_AUTO);

	return queue;
}

void ff_blocking_queue_delete(struct ff_blocking_queue *queue)
{
	ff_event_delete(queue->event);
	ff_queue_delete(queue->simple_queue);
	ff_free(queue);
}

void *ff_blocking_queue_get(struct ff_blocking_queue *queue)
{
	void *data;
	int is_empty;

	for (;;)
	{
		is_empty = ff_queue_is_empty(queue->simple_queue);
		if (!is_empty)
		{
			break;
		}
		ff_event_wait(queue->event);
	}
	data = ff_queue_front(queue->simple_queue);
	ff_queue_pop(queue->simple_queue);
	is_empty = ff_queue_is_empty(queue->simple_queue);
	if (!is_empty)
	{
		ff_event_set(queue->event);
	}
	return data;
}

void ff_blocking_queue_put(struct ff_blocking_queue *queue, void *data)
{
	int is_empty = ff_queue_is_empty(queue->simple_queue);
	ff_queue_push(queue->simple_queue, data);
	if (is_empty)
	{
		ff_event_set(queue->event);
	}
}
