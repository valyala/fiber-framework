#include "private/ff_common.h"

#include "private/ff_stream_pipe.h"
#include "private/ff_pipe.h"

static void delete_pipe(void *ctx)
{
	struct ff_pipe *pipe;

	pipe = (struct ff_pipe *) ctx;
	ff_pipe_delete(pipe);
}

static enum ff_result read_from_pipe(void *ctx, void *buf, int len)
{
	struct ff_pipe *pipe;
	enum ff_result result;

	ff_assert(len >= 0);

	pipe = (struct ff_pipe *) ctx;
	result = ff_pipe_read(pipe, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while reading from the pipe=%p to the buf=%p, len=%d. See previous messages for more info", pipe, buf, len);
	}
	return result;
}

static enum ff_result write_to_pipe(void *ctx, const void *buf, int len)
{
	struct ff_pipe *pipe;
	enum ff_result result;

	ff_assert(len >= 0);

	pipe = (struct ff_pipe *) ctx;
	result = ff_pipe_write(pipe, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"error while writing to the pipe=%p from the buf=%p, len=%d. See previous messages for more info", pipe, buf, len);
	}
	return result;
}

static enum ff_result flush_pipe(void *ctx)
{
	/* the pipe is automatically flushed, so there is no need in its flushing anymore */

	return FF_SUCCESS;
}

static void disconnect_pipe(void *ctx)
{
	struct ff_pipe *pipe;

	pipe = (struct ff_pipe *) ctx;
	ff_pipe_disconnect(pipe);
}

static const struct ff_stream_vtable pipe_stream_vtable =
{
	delete_pipe,
	read_from_pipe,
	write_to_pipe,
	flush_pipe,
	disconnect_pipe
};

void ff_stream_pipe_create_pair(int buffer_size, struct ff_stream **stream1, struct ff_stream **stream2)
{
	struct ff_pipe *pipe1;
	struct ff_pipe *pipe2;
	
	ff_assert(buffer_size >= 0);
	ff_assert(stream1 != NULL);
	ff_assert(stream2 != NULL);

	ff_pipe_create_pair(buffer_size, &pipe1, &pipe2);

	*stream1 = ff_stream_create(&pipe_stream_vtable, pipe1);
	*stream2 = ff_stream_create(&pipe_stream_vtable, pipe2);
}
