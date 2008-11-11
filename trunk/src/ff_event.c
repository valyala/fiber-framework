#include "private/ff_common.h"

#include "private/ff_event.h"
#include "private/ff_core.h"
#include "private/ff_fiber.h"
#include "private/ff_stack.h"

struct ff_event
{
	struct ff_stack *pending_fibers;
	enum ff_event_type event_type;
	int is_set;
};

static void cancel_event_wait(struct ff_fiber *fiber, void *ctx)
{
	struct ff_event *event;
	enum ff_result result;
	
	event = (struct ff_event *) ctx;
	result = ff_stack_remove_entry(event->pending_fibers, fiber);
	if (result == FF_SUCCESS)
	{
		ff_core_schedule_fiber(fiber);
	}
}

struct ff_event *ff_event_create(enum ff_event_type event_type)
{
	struct ff_event *event;

	event = (struct ff_event *) ff_malloc(sizeof(*event));
	event->pending_fibers = ff_stack_create();
	event->event_type = event_type;
	event->is_set = 0;

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
			int is_empty;

			is_empty = ff_stack_is_empty(event->pending_fibers);
			if (is_empty)
			{
				event->is_set = 1;
				break;
			}

			ff_stack_top(event->pending_fibers, (const void **) &fiber);
			ff_stack_pop(event->pending_fibers);
			ff_core_schedule_fiber(fiber);
			if (event->event_type == FF_EVENT_AUTO)
			{
				/* there is no need to set the event in this case,
				 * because autoreset event must be reset if somebody has been woked up.
				 */
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
	if (!event->is_set)
	{
		struct ff_fiber *current_fiber;

		current_fiber = ff_fiber_get_current();
		ff_stack_push(event->pending_fibers, current_fiber);
		ff_core_yield_fiber();
		/* the event can be already reset (event->is_set == 0) at this moment:
		 * f1: ff_event_reset(); // event->is_set = 0;
		 * f2: enter ff_event_wait(); // f2 has been blocked
		 * f1: ff_event_set(); // f2 scheduled for execution (but not executed), event->is_set = 1
		 * f1: ff_event_reset(); // event->is_set = 0, but f2() will be executed at the next step.
		 * f2: exit ff_event_wait(); // is is at this point and (event->is_set == 0).
		 */
	}
	else if (event->event_type == FF_EVENT_AUTO)
	{
		/* autoreset event should be reset if it is set in order to
		 * block subsequent ff_event_wait() calls.
		 */
		event->is_set = 0;
	}
}

enum ff_result ff_event_wait_with_timeout(struct ff_event *event, int timeout)
{
	enum ff_result result = FF_SUCCESS;

	ff_assert(timeout > 0);

	if (!event->is_set)
	{
		struct ff_fiber *current_fiber;
		struct ff_core_timeout_operation_data *timeout_operation_data;

		current_fiber = ff_fiber_get_current();
		ff_stack_push(event->pending_fibers, current_fiber);
		timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_event_wait, event);
		ff_core_yield_fiber();
		result = ff_core_deregister_timeout_operation(timeout_operation_data);
		/* the event can be already reset (event->is_set == 0) at this moment:
		 * f1: ff_event_reset(); // event->is_set = 0;
		 * f2: enter ff_event_wait(); // f2 has been blocked
		 * f1: ff_event_set(); // f2 scheduled for execution (but not executed), event->is_set = 1
		 * f1: ff_event_reset(); // event->is_set = 0, but f2() will be executed at the next step.
		 * f2: exit ff_event_wait(); // is is at this point and (event->is_set == 0).
		 */
	}
	else if (event->event_type == FF_EVENT_AUTO)
	{
		/* autoreset event should be reset if it is set in order to
		 * block subsequent ff_event_wait() calls.
		 */
		event->is_set = 0;
	}
	return result;
}

int ff_event_is_set(struct ff_event *event)
{
	return event->is_set;
}
