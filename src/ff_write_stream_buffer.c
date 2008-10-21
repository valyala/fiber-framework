#include "private/ff_common.h"

#include "private/ff_write_stream_buffer.h"

struct ff_write_stream_buffer
{
	ff_write_stream_func write_func;
	void *func_ctx;
	char *buf;
	int capacity;
	int start_pos;
};

struct ff_write_stream_buffer *ff_write_stream_buffer_create(ff_write_stream_func write_func, void *func_ctx, int capacity)
{
	struct ff_write_stream_buffer *buffer;

	ff_assert(capacity > 0);

	buffer = (struct ff_write_stream_buffer *) ff_malloc(sizeof(*buffer));
	buffer->write_func = write_func;
	buffer->func_ctx = func_ctx;
	buffer->buf = (char *) ff_calloc(capacity, sizeof(buffer->buf[0]));
	buffer->capacity = capacity;
	buffer->start_pos = 0;

	return buffer;
}

void ff_write_stream_buffer_delete(struct ff_write_stream_buffer *buffer)
{
	ff_free(buffer->buf);
	ff_free(buffer);
}

enum ff_result ff_write_stream_buffer_write(struct ff_write_stream_buffer *buffer, const void *buf, int len)
{
	char *char_buf;
	enum ff_result result = FF_FAILURE;

	ff_assert(len >= 0);

	char_buf = (char *) buf;
	while (len > 0)
	{
		int bytes_written;
		int free_bytes_cnt;

		ff_assert(buffer->capacity > 0);
		ff_assert(buffer->start_pos >= 0);
		ff_assert(buffer->start_pos <= buffer->capacity);

		if (buffer->capacity == buffer->start_pos)
		{
			/* the buffer is full, so flush its contents to the underlying stream */
			result = ff_write_stream_buffer_flush(buffer);
			if (result == FF_FAILURE)
			{
				goto end;
			}
			ff_assert(buffer->start_pos == 0);

			/* write data directly from the char_buf to the underlying stream
			 * until len is greater than buffer capacity. This allows to avoid superflous
			 * copying of data into the buffer before flushing it to the underlying stream.
			 */
			while (len >= buffer->capacity)
			{
				bytes_written = buffer->write_func(buffer->func_ctx, char_buf, len);
				if (bytes_written == -1)
				{
					goto end;
				}
				ff_assert(bytes_written > 0);
				ff_assert(bytes_written <= len);
				char_buf += bytes_written;
				len -= bytes_written;
			}
			if (len == 0)
			{
				/* all requested data has been written directly to the underlying stream */
				break;
			}
		}
		ff_assert(buffer->start_pos < buffer->capacity);

		/* there is the room in the buffer for data. Copy it to the buffer */
		free_bytes_cnt = buffer->capacity - buffer->start_pos;
		ff_assert(free_bytes_cnt > 0);
		bytes_written = (free_bytes_cnt > len) ? len : free_bytes_cnt;
		memcpy(buffer->buf + buffer->start_pos, char_buf, bytes_written);

		buffer->start_pos += bytes_written;
		char_buf += bytes_written;
		len -= bytes_written;
	}
	result = FF_SUCCESS;

end:
	return result;
}


enum ff_result ff_write_stream_buffer_flush(struct ff_write_stream_buffer *buffer)
{
	int total_bytes_written = 0;
	int bytes_to_write;
	enum ff_result result = FF_FAILURE;

	ff_assert(buffer->capacity > 0);
	ff_assert(buffer->start_pos >= 0);
	ff_assert(buffer->start_pos <= buffer->capacity);

	bytes_to_write = buffer->start_pos;
	while (bytes_to_write > 0)
	{
		int bytes_written;

		bytes_written = buffer->write_func(buffer->func_ctx, buffer->buf + total_bytes_written, bytes_to_write);
		if (bytes_written == -1)
		{
			goto end;
		}
		ff_assert(bytes_written > 0);
		ff_assert(bytes_written <= bytes_to_write);

		bytes_to_write -= bytes_written;
		total_bytes_written += bytes_written;
	}
	buffer->start_pos = 0;
	result = FF_SUCCESS;

end:
	return result;
}
