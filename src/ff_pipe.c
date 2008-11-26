#include "private/ff_common.h"
#include "private/ff_pipe.h"
#include "private/ff_loopback.h"

struct ff_pipe
{
	struct ff_loopback *read_loopback;
	struct ff_loopback *write_loopback;
	int *ref_cnt;
};

static struct ff_pipe *create_pipe(struct ff_loopback *read_loopback, struct ff_loopback *write_loopback, int *ref_cnt)
{
	struct ff_pipe *pipe;

	pipe = (struct ff_pipe *) ff_malloc(sizeof(*pipe));
	pipe->read_loopback = read_loopback;
	pipe->write_loopback = write_loopback;
	pipe->ref_cnt = ref_cnt;

	return pipe;
}

void ff_pipe_create_pair(int buffer_size, struct ff_pipe **pipe1, struct ff_pipe **pipe2)
{
	struct ff_loopback *loopback1;
	struct ff_loopback *loopback2;
	int *ref_cnt;

	ff_assert(buffer_size > 1);

	loopback1 = ff_loopback_create(buffer_size);
	loopback2 = ff_loopback_create(buffer_size);
	ref_cnt = (int *) ff_malloc(sizeof(*ref_cnt));

	*ref_cnt = 2;
	*pipe1 = create_pipe(loopback1, loopback2, ref_cnt);
	*pipe2 = create_pipe(loopback2, loopback1, ref_cnt);
}

void ff_pipe_delete(struct ff_pipe *pipe)
{
	ff_assert(pipe != NULL);
	ff_assert(*pipe->ref_cnt > 0);
	ff_assert(*pipe->ref_cnt <= 2);

	(*pipe->ref_cnt)--;
	if (*pipe->ref_cnt == 0)
	{
		ff_loopback_delete(pipe->read_loopback);
		ff_loopback_delete(pipe->write_loopback);
		ff_free(pipe->ref_cnt);
	}
	ff_free(pipe);
}

enum ff_result ff_pipe_read(struct ff_pipe *pipe, void *buf, int len)
{
	enum ff_result result;

	ff_assert(pipe != NULL);

	result = ff_loopback_read(pipe->read_loopback, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot read data from the pipe=%p to the buf=%p, len=%d. See previous messages for more info", pipe, buf, len);
	}
	return result;
}

enum ff_result ff_pipe_write(struct ff_pipe *pipe, const void *buf, int len)
{
	enum ff_result result;

	ff_assert(pipe != NULL);

	result = ff_loopback_write(pipe->write_loopback, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot write data to the pipe=%p from the buf=%p, len=%d. See previous messages for more info", pipe, buf, len);
	}
	return result;
}

void ff_pipe_disconnect(struct ff_pipe *pipe)
{
	ff_assert(pipe != NULL);

	ff_loopback_disconnect(pipe->write_loopback);
	ff_loopback_disconnect(pipe->read_loopback);
}
