#include "private/ff_common.h"

#include "private/ff_event.h"
#include "private/ff_core.h"
#include "private/ff_fiber.h"
#include "private/ff_stack.h"

struct ff_event
{
	int is_set;
	enum ff_event_type event_type;
	struct ff_stack *pending_fibers;
};

static void cancel_event_wait(struct ff_fiber *fiber, void *ctx)
{
	struct ff_event *event = (struct ff_event *) ctx;
	int is_removed = ff_stack_remove_entry(event->pending_fibers, fiber);
	if (is_removed)
	{
		ff_core_schedule_fiber(fiber);
	}
}

struct ff_event *ff_event_create(enum ff_event_type event_type)
{
	struct ff_event *event = (struct ff_event *) ff_malloc(sizeof(*event));

	event->is_set = 0;
	event->event_type = event_type;
	event->pending_fibers = ff_stack_create();

	return event;
}

void ff_event_delete(struct ff_event *event)
{
	ff_stack_delete(event->pending_fibers);
	ff_free(event);
}

void ff_event_set(struct ff_event *event)
{
	if (!event->is_set)
	{
		for (;;)
		{
			struct ff_fiber *fiber;
			int is_empty = ff_stack_is_empty(event->pending_fibers);
			if (is_empty)
			{
				event->is_set = 1;
				break;
			}

			fiber = (struct ff_fiber *) ff_stack_top(event->pending_fibers);
			ff_stack_pop(event->pending_fibers);
			ff_core_schedule_fiber(fiber);
			if (event->event_type == FF_EVENT_AUTO)
			{
				/* event->is_set = 1 skipped here intentionally */
				break;
			}
		}
	}
}

void ff_event_reset(struct ff_event *event)
{
	event->is_set = 0;
}

void ff_event_wait(struct ff_event *event)
{
	int is_success;

	is_success = ff_event_wait_with_timeout(event, 0);
	ff_assert(is_success);
}

int ff_event_wait_with_timeout(struct ff_event *event, int timeout)
{
	int is_success = 1;
	if (!event->is_set)
	{
		struct ff_fiber *current_fiber = ff_core_get_current_fiber();
		ff_stack_push(event->pending_fibers, current_fiber);
		is_success = ff_core_do_timeout_operation(timeout, cancel_event_wait, event);
	}
	return is_success;
}

int ff_event_is_set(struct ff_event *event)
{
	return event->is_set;
}
