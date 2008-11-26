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

struct ff_stream *ff_stream_pipe_create(struct ff_pipe *pipe)
{
	struct ff_stream *stream;

	stream = ff_stream_create(&pipe_stream_vtable, pipe);
	return stream;
}
