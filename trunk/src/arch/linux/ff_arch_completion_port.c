#include "private/arch/ff_arch_completion_port.h"
#include "private/ff_stack.h"
#include "private/arch/ff_arch_mutex.h"
#include "ff_linux_completion_port.h"
#include "ff_linux_error_check.h"

#include <sys/epoll.h>

struct ff_arch_completion_port
{
	int epoll_fd;
	struct ff_stack *pending_events;
	struct ff_arch_mutex *mutex;
};

static const int EPOLL_FD_SIZE = 10;

struct ff_arch_completion_port *ff_arch_completion_port_create(int concurrency)
{
	struct ff_arch_completion_port *completion_port;

	completion_port = (struct ff_arch_completion_port *) ff_malloc(sizeof(*completion_port));
	completion_port->epoll_fd = epoll_create(EPOLL_FD_SIZE);
	ff_linux_fatal_error_check(completion_port->epoll_fd != -1, "cannot create epoll file descriptor");
	completion_port->pending_events = ff_stack_create();
	completion_port->pending_events_mutex = ff_arch_mutex_create();

	return completion_port;
}

void ff_arch_completion_port_delete(struct ff_arch_completion_port *completion_port)
{
	int rv;

	ff_arch_mutex_delete(completion_port->pending_events_mutex);
	ff_stack_delete(completion_port->pending_events);
	rv = close(completion_port->epoll_fd);
	ff_assert(rv == 0);
	ff_free(completion_port);
}

void *ff_arch_completion_port_get(struct ff_arch_completion_port *completion_port)
{
	void *data;
	int is_empty;

	ff_arch_mutex_lock(completion_port->pending_events_mutex);
	is_empty = ff_stack_is_empty(completion_port->pending_events);
	if (is_empty)
	{
		int events_cnt;
		int i;
		struct epoll_event events[EPOLL_FD_SIZE];

		ff_arch_mutex_unlock(completion_port->pending_events_mutex);
again:
    	events_cnt = epoll_wait(completion_port->epoll_fd, events, EPOLL_FD_SIZE, -1);
    	if (events_cnt == -1)
    	{
    		ff_linux_fatal_error_check(errno == EINTR, "epoll_wait() failed");
    		goto again;
    	}
    	ff_linux_fatal_error_check(events_cnt > 0, "epoll_wait() unexpectedly returned 0");

    	ff_arch_mutex_lock(completion_port->pending_events_mutex);
    	for (i = 1; i < events_cnt; i++)
    	{
    		data = events[i].data.ptr;
    		ff_stack_push(completion_port->pending_events, data);
    	}
    	ff_arch_mutex_unlock(completion_port->pending_events_mutex);
    	data = events[0].data.ptr;
    }
    else
    {
    	ff_stack_pop(completion_port->pending_events, &data);
    	ff_arch_mutex_unlock(completion_port->pending_events_mutex);
    }

	return data;
}

void ff_arch_completion_port_put(struct ff_arch_completion_port *completion_port, void *data)
{
	ff_arch_mutex_lock(completion_port->pending_events_mutex);
	ff_stack_push(completion_port->pending_events, data);
	ff_arch_mutex_unlock(completion_port->pending_events_mutex);
}

void ff_linux_completion_port_register_operation(struct ff_arch_completion_port *completion_port, int fd, enum ff_linux_completion_port_operation_type operation_type, void *data)
{
	int rv;
	struct epoll_event event;
	__uint32_t epoll_operation;

	epoll_operation = (operation_type == FF_COMPLETION_PORT_OPERATION_READ) ? EPOLLIN : EPOLLOUT;
	event.events = EPOLLET | EPOLLONESHOT | epoll_operation;
	event.data = data;

	rv = epoll_ctl(completion_port->epoll_fd, EPOLL_CTL_ADD, fd, &event);
	ff_linux_fatal_error_check(rv != -1, "epoll_ctl() failed");
}
