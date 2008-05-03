#ifndef FF_EVENT_PUBLIC
#define FF_EVENT_PUBLIC

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
struct ff_event *ff_event_create(enum ff_event_type event_type);

/**
 * @public
 * deletes the event
 */
void ff_event_delete(struct ff_event *event);

/**
 * @public
 * sets the given event
 */
void ff_event_set(struct ff_event *event);

/**
 * @public
 * resets the given event
 */
void ff_event_reset(struct ff_event *event);

/**
 * @public
 * waits while the given event will be set
 */
void ff_event_wait(struct ff_event *event);

/**
 * @public
 * waits while the given event will be set during the timeout;
 * Returns 0 if the event wasn't set. Otherwise returns 1.
 */
int ff_event_wait_with_timeout(struct ff_event *event, int timeout);

/**
 * @public
 * Returns 0 if event isn't set, otherwise returns non-zero.
 */
int ff_event_is_set(struct ff_event *event);

#ifdef __cplusplus
}
#endif

#endif
