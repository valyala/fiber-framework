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

	buffer = (struct ff_write_stream_buffer *) ff_malloc(sizeof(*buffer));
	buffer->write_func = write_func;
	buffer->func_ctx = func_ctx;
	buffer->buf = (char *) ff_malloc(capacity);
	buffer->capacity = capacity;
	buffer->start_pos = 0;

	return buffer;
}

void ff_write_stream_buffer_delete(struct ff_write_stream_buffer *buffer)
{
	ff_free(buffer->buf);
	ff_free(buffer);
}

int ff_write_stream_buffer_write(struct ff_write_stream_buffer *buffer, const void *buf, int len)
{
	char *char_buf;
	int total_bytes_written;

	total_bytes_written = len;
	char_buf = (char *) buf;
	for (;;)
	{
		int bytes_written;
		int free_bytes_cnt;

		if (buffer->capacity == buffer->start_pos)
		{
			bytes_written = ff_write_stream_buffer_flush(buffer);
			if (bytes_written == -1)
			{
				total_bytes_written = -1;
				goto end;
			}
			ff_assert(buffer->start_pos == 0);

			while (len >= buffer->capacity)
			{
				bytes_written = buffer->write_func(buffer->func_ctx, char_buf, len);
				if (bytes_written == -1)
				{
					total_bytes_written = -1;
					goto end;
				}

				char_buf += bytes_written;
				len -= bytes_written;
			}
		}
		if (len == 0)
		{
			goto end;
		}

		free_bytes_cnt = buffer->capacity - buffer->start_pos;
		ff_assert(free_bytes_cnt > 0);
		bytes_written = (free_bytes_cnt > len) ? len : free_bytes_cnt;
		memcpy(buffer->buf + buffer->start_pos, char_buf, bytes_written);

		buffer->start_pos += bytes_written;
		
		char_buf += bytes_written;
		len -= bytes_written;
	}

end:
	return total_bytes_written;
}


int ff_write_stream_buffer_flush(struct ff_write_stream_buffer *buffer)
{
	int total_bytes_written = 0;

	while (buffer->start_pos > 0)
	{
		int bytes_written;

		bytes_written = buffer->write_func(buffer->func_ctx, buffer->buf + total_bytes_written, buffer->start_pos);
		if (bytes_written == -1)
		{
			total_bytes_written = -1;
			goto end;
		}

		buffer->start_pos -= bytes_written;
		ff_assert(buffer->start_pos >= 0);

		total_bytes_written += bytes_written;
	}

end:
	return total_bytes_written;
}
