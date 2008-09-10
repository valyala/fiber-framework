#include "private/ff_common.h"

#include "private/ff_container.h"

struct ff_container_entry
{
	struct ff_container_entry *next;
	struct ff_container_entry **prev_ptr;
	const void *data;
};

struct ff_container
{
	struct ff_container_entry *head;
};

struct ff_container *ff_container_create()
{
	struct ff_container *container;

	container = (struct ff_container *) ff_malloc(sizeof(*container));
	container->head = NULL;

	return container;
}

void ff_container_delete(struct ff_container *container)
{
	ff_assert(container->head == NULL);

	ff_free(container);
}

struct ff_container_entry *ff_container_add_entry(struct ff_container *container, const void *data)
{
	struct ff_container_entry *entry;

	entry = (struct ff_container_entry *) ff_malloc(sizeof(*entry));
	entry->next = container->head;
	entry->prev_ptr = &container->head;
	entry->data = data;
	if (container->head != NULL)
	{
		container->head->prev_ptr = &entry->next;
	}
	container->head = entry;

	return entry;
}

void ff_container_remove_entry(struct ff_container_entry *entry)
{
	ff_assert(*entry->prev_ptr == entry);

	*entry->prev_ptr = entry->next;
	if (entry->next != NULL)
	{
		ff_assert(entry->next->prev_ptr == &entry->next);
		entry->next->prev_ptr = entry->prev_ptr;
	}
	ff_free(entry);
}

void ff_container_for_each(struct ff_container *container, ff_container_for_each_func for_each_func, void *ctx)
{
	struct ff_container_entry *entry;

	entry = container->head;
	while (entry != NULL)
	{
		for_each_func(entry->data, ctx);
		entry = entry->next;
	}
}

int ff_container_is_empty(struct ff_container *container)
{
	int is_empty;

	is_empty = (container->head == NULL) ? 1 : 0;
	return is_empty;
}
