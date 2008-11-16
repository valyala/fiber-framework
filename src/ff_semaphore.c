#include "private/ff_common.h"

#include "private/ff_semaphore.h"
#include "private/ff_event.h"

struct ff_semaphore
{
	struct ff_event *event;
	int value;
};

struct ff_semaphore *ff_semaphore_create(int value)
{
	struct ff_semaphore *semaphore;

	ff_assert(value >= 0);

	semaphore = (struct ff_semaphore *) ff_malloc(sizeof(*semaphore));
	semaphore->event = ff_event_create(FF_EVENT_AUTO);
	semaphore->value = value;

	return semaphore;
}

void ff_semaphore_delete(struct ff_semaphore *semaphore)
{
	ff_assert(semaphore->value >= 0);

	ff_event_delete(semaphore->event);
	ff_free(semaphore);
}

void ff_semaphore_up(struct ff_semaphore *semaphore)
{
	ff_assert(semaphore->value >= 0);

	semaphore->value++;
	if (semaphore->value == 1)
	{
		ff_event_set(semaphore->event);
	}
}

void ff_semaphore_down(struct ff_semaphore *semaphore)
{
	struct ff_event *event;

	ff_assert(semaphore->value >= 0);

	event = semaphore->event;
	while (semaphore->value == 0)
	{
		ff_event_wait(event);
	}
	semaphore->value--;
	if (semaphore->value > 0)
	{
		ff_event_set(event);
	}
}

enum ff_result ff_semaphore_down_with_timeout(struct ff_semaphore *semaphore, int timeout)
{
	struct ff_event *event;
	enum ff_result result = FF_SUCCESS;

	ff_assert(semaphore->value >= 0);
	ff_assert(timeout > 0);

	event = semaphore->event;
	while (semaphore->value == 0)
	{
		result = ff_event_wait_with_timeout(event, timeout);
		if (result != FF_SUCCESS)
		{
			goto end;
		}
	}
	semaphore->value--;
	if (semaphore->value > 0)
	{
		ff_event_set(event);
	}

end:
	return result;
}
