#ifndef FF_EVENT_PUBLIC_H
#define FF_EVENT_PUBLIC_H

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * the opaque event structure
 */
struct ff_event;

/**
 * @public
 * possible event types
 */
enum ff_event_type
{
	/* manual reset event */
	FF_EVENT_MANUAL,

	/* auto-reset event */
	FF_EVENT_AUTO
};

/**
 * @public
 * creates the event with the given type.
 * The newly created event is in the reset state.
 * Always returns correct result.
 */
FF_API struct ff_event *ff_event_create(enum ff_event_type event_type);

/**
 * @public
 * deletes the event
 */
FF_API void ff_event_delete(struct ff_event *event);

/**
 * @public
 * sets the given event
 */
FF_API void ff_event_set(struct ff_event *event);

/**
 * @public
 * resets the given event
 */
FF_API void ff_event_reset(struct ff_event *event);

/**
 * @public
 * waits while the given event will be set
 */
FF_API void ff_event_wait(struct ff_event *event);

/**
 * @public
 * waits while the given event will be set during the timeout;
 * Returns FF_FAILURE if the event wasn't set. Otherwise returns FF_SUCCESS.
 */
FF_API enum ff_result ff_event_wait_with_timeout(struct ff_event *event, int timeout);

/**
 * @public
 * Returns 0 if event isn't set, otherwise returns non-zero.
 */
FF_API int ff_event_is_set(struct ff_event *event);

#ifdef __cplusplus
}
#endif

#endif
