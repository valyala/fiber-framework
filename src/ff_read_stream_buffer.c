#include "private/ff_common.h"

#include "private/ff_read_stream_buffer.h"

struct ff_read_stream_buffer
{
	ff_read_stream_func read_func;
	void *func_ctx;
	char *buf;
	int capacity;
	int size;
	int start_pos;
};

struct ff_read_stream_buffer *ff_read_stream_buffer_create(ff_read_stream_func read_func, void *func_ctx, int capacity)
{
	struct ff_read_stream_buffer *buffer;

	ff_assert(capacity > 0);

	buffer = (struct ff_read_stream_buffer *) ff_malloc(sizeof(*buffer));
	buffer->read_func = read_func;
	buffer->func_ctx = func_ctx;
	buffer->buf = (char *) ff_malloc(capacity);
	buffer->capacity = capacity;
	buffer->size = 0;
	buffer->start_pos = 0;

	return buffer;
}

void ff_read_stream_buffer_delete(struct ff_read_stream_buffer *buffer)
{
	ff_free(buffer->buf);
	ff_free(buffer);
}

int ff_read_stream_buffer_read(struct ff_read_stream_buffer *buffer, void *buf, int len)
{
	char *char_buf;
	int total_bytes_read = 0;

	ff_assert(len >= 0);

	char_buf = (char *) buf;
	for (;;)
	{
		int bytes_read;

		if (buffer->size == 0)
		{
			while (len >= buffer->capacity)
			{
				bytes_read = buffer->read_func(buffer->func_ctx, char_buf, len);
				if (bytes_read == -1)
				{
					total_bytes_read = -1;
					goto end;
				}
				if (bytes_read == 0)
				{
					goto end;
				}

				char_buf += bytes_read;
				total_bytes_read += bytes_read;
				len -= bytes_read;
			}
			if (len == 0)
			{
				goto end;
			}

			bytes_read = buffer->read_func(buffer->func_ctx, buffer->buf, buffer->capacity);
			if (bytes_read == -1)
			{
				total_bytes_read = -1;
				goto end;
			}
			if (bytes_read == 0)
			{
				goto end;
			}

			buffer->size = bytes_read;
			buffer->start_pos = 0;
		}
		if (len == 0)
		{
			goto end;
		}

		bytes_read = len > buffer->size ? buffer->size : len;
		ff_assert(bytes_read > 0);
		memcpy(char_buf, buffer->buf + buffer->start_pos, bytes_read);

		buffer->start_pos += bytes_read;
		buffer->size -= bytes_read;

		char_buf += bytes_read;
		total_bytes_read += bytes_read;
		len -= bytes_read;
	}

end:
	return total_bytes_read;
}
