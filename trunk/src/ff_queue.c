#include "private/ff_common.h"

#include "private/ff_queue.h"
#include "private/ff_lock.h"

struct queue_entry
{
	struct queue_entry *next;
	void *data;
};

struct ff_queue
{
	struct ff_lock *lock;
	struct queue_entry *front;
	struct queue_entry **back_ptr;
};

struct ff_queue *ff_queue_create()
{
	struct ff_queue *queue = (struct ff_queue *) ff_malloc(sizeof(*queue));
	queue->lock = ff_lock_create();
	queue->front = NULL;
	queue->back_ptr = &queue->front;
	return queue;
}

void ff_queue_delete(struct ff_queue *queue)
{
	ff_assert(queue->front == NULL);
	ff_assert(queue->back_ptr == &queue->front);

	ff_lock_delete(queue->lock);
	ff_free(queue);
}

void ff_queue_push(struct ff_queue *queue, void *data)
{
	struct queue_entry *entry = (struct queue_entry *) ff_malloc(sizeof(*entry));
	entry->next = NULL;
	entry->data = data;

	ff_lock_lock(queue->lock);
	*queue->back_ptr = entry;
	queue->back_ptr = &entry->next;
	ff_lock_unlock(queue->lock);
}

int ff_queue_is_empty(struct ff_queue *queue)
{
	int is_empty = (queue->front == NULL) ? 1 : 0;
	return is_empty;
}

void *ff_queue_front(struct ff_queue *queue)
{
	ff_assert(queue->front != NULL);

	return queue->front->data;
}

void ff_queue_pop(struct ff_queue *queue)
{
	struct queue_entry *entry;

	ff_assert(queue->front != NULL);

	ff_lock_lock(queue->lock);
	entry = queue->front;
	queue->front = entry->next;
	if (queue->back_ptr == &entry->next)
	{
		queue->back_ptr = &queue->front;
	}
	ff_lock_unlock(queue->lock);

	ff_free(entry);
}

int ff_queue_remove_entry(struct ff_queue *queue, void *data)
{
	int is_removed = 0;
	struct queue_entry **entry_ptr;
	struct queue_entry *entry;

	ff_lock_lock(queue->lock);
	entry_ptr = &queue->front;
	entry = queue->front;
	while (entry != NULL)
	{
		if (entry->data == data)
		{
			*entry_ptr = entry->next;
			if (queue->back_ptr == &entry->next)
			{
				queue->back_ptr = entry_ptr;
			}
			ff_free(entry);
			is_removed = 1;
			break;
		}
		entry_ptr = &entry->next;
		entry = entry->next;
	}
	ff_lock_unlock(queue->lock);

	return is_removed;
}

void ff_queue_for_each(struct ff_queue *queue, ff_queue_for_each_func for_each_func, void *ctx)
{
	struct queue_entry *entry;

	ff_lock_lock(queue->lock);
	entry = queue->front;
	while (entry != NULL)
	{
		for_each_func(entry->data, ctx);
		entry = entry->next;
	}
	ff_lock_unlock(queue->lock);
}
