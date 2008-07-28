#include "private/arch/ff_arch_completion_port.h"
#include "private/ff_stack.h"
#include "private/arch/ff_arch_mutex.h"
#include "ff_linux_completion_port.h"
#include "ff_linux_error_check.h"

#include <sys/epoll.h>
#include <unistd.h>

/* workaround of debian bug #261541 (missing EPOLLONESHOT declaration in sys/epoll.h) */
#ifndef EPOLLONESHOT
#	define EPOLLONESHOT (1 << 30)
#endif

static const int EPOLL_CAPACITY = 10;

struct ff_arch_completion_port
{
	int epoll_fd;
	int rd_pipe;
	int wr_pipe;
	struct ff_stack *pending_events;
	struct ff_arch_mutex *pending_events_mutex;
};

struct ff_arch_completion_port *ff_arch_completion_port_create(int concurrency)
{
	struct ff_arch_completion_port *completion_port;
	int pipe_fds[2];
	int rv;
	struct epoll_event event;

	rv = pipe(pipe_fds);
	ff_linux_fatal_error_check(rv != -1, "cannot create pipe");

	completion_port = (struct ff_arch_completion_port *) ff_malloc(sizeof(*completion_port));
	completion_port->epoll_fd = epoll_create(EPOLL_CAPACITY);
	ff_linux_fatal_error_check(completion_port->epoll_fd != -1, "cannot create epoll file descriptor");
	completion_port->rd_pipe = pipe_fds[0];
	completion_port->wr_pipe = pipe_fds[1];
	completion_port->pending_events = ff_stack_create();
	completion_port->pending_events_mutex = ff_arch_mutex_create();

	event.data.ptr = completion_port;
	event.events = EPOLLIN;
	rv = epoll_ctl(completion_port->epoll_fd, EPOLL_CTL_ADD, completion_port->rd_pipe, &event);
	ff_linux_fatal_error_check(rv != -1, "epoll_ctl(rd_pipe) failed");

	return completion_port;
}

void ff_arch_completion_port_delete(struct ff_arch_completion_port *completion_port)
{
	int rv;

	ff_arch_mutex_delete(completion_port->pending_events_mutex);
	ff_stack_delete(completion_port->pending_events);
	rv = close(completion_port->wr_pipe);
	ff_assert(rv == 0);
	rv = close(completion_port->rd_pipe);
	ff_assert(rv == 0);
	rv = close(completion_port->epoll_fd);
	ff_assert(rv == 0);
	ff_free(completion_port);
}

void ff_arch_completion_port_get(struct ff_arch_completion_port *completion_port, const void **data)
{
	int is_empty;

	ff_arch_mutex_lock(completion_port->pending_events_mutex);
	is_empty = ff_stack_is_empty(completion_port->pending_events);
	if (is_empty)
	{
		int events_cnt;
		int i;
		struct epoll_event events[EPOLL_CAPACITY];

		ff_arch_mutex_unlock(completion_port->pending_events_mutex);
		for (;;)
		{
	    	events_cnt = epoll_wait(completion_port->epoll_fd, events, EPOLL_CAPACITY, -1);
	    	if (events_cnt != -1)
	    	{
	    		break;
	    	}
    		ff_linux_fatal_error_check(errno == EINTR, "epoll_wait() failed");
    	}
    	ff_linux_fatal_error_check(events_cnt > 0, "epoll_wait() unexpectedly returned 0");

    	ff_arch_mutex_lock(completion_port->pending_events_mutex);
    	for (i = 0; i < events_cnt; i++)
    	{
    		const void *tmp;

    		tmp = events[i].data.ptr;
    		if (tmp == completion_port)
    		{
    			/* read data from pipe */
				ssize_t bytes_read;

				for (;;)
				{
					bytes_read = read(completion_port->rd_pipe, &tmp, sizeof(tmp));
					if (bytes_read != -1)
					{
						break;
					}
					ff_linux_fatal_error_check(errno == EINTR, "read(rd_pipe) failed");
				}
				ff_linux_fatal_error_check(bytes_read == sizeof(tmp), "error when reading from the pipe");
    		}
    		ff_stack_push(completion_port->pending_events, tmp);
    	}
    }

	ff_stack_top(completion_port->pending_events, data);
	ff_stack_pop(completion_port->pending_events);
	ff_arch_mutex_unlock(completion_port->pending_events_mutex);
}

void ff_arch_completion_port_put(struct ff_arch_completion_port *completion_port, const void *data)
{
	ssize_t bytes_written;

	for (;;)
	{
		bytes_written = write(completion_port->wr_pipe, &data, sizeof(data));
		if (bytes_written != -1)
		{
			break;
		}
		ff_lunux_fatal_error_check(errno == EINTR, "write(wr_pipe) failed");
	}
	ff_linux_fatal_error_check(bytes_written == sizeof(data), "error when writing to pipe");
}

void ff_linux_completion_port_register_operation(struct ff_arch_completion_port *completion_port, int fd, enum ff_linux_completion_port_operation_type operation_type, const void *data)
{
	int rv;
	struct epoll_event event;
	__uint32_t epoll_operation;

	epoll_operation = (operation_type == FF_COMPLETION_PORT_OPERATION_READ) ? EPOLLIN : EPOLLOUT;
	event.events = EPOLLET | EPOLLONESHOT | epoll_operation;
	event.data.ptr = (void *) data;

	rv = epoll_ctl(completion_port->epoll_fd, EPOLL_CTL_ADD, fd, &event);
	ff_linux_fatal_error_check(rv != -1, "epoll_ctl() failed");
}
