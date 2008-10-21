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
	buffer->buf = (char *) ff_calloc(capacity, sizeof(buffer->buf[0]));
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

enum ff_result ff_read_stream_buffer_read(struct ff_read_stream_buffer *buffer, void *buf, int len)
{
	char *char_buf;
	enum ff_result result = FF_FAILURE;

	ff_assert(len >= 0);

	char_buf = (char *) buf;
	while (len > 0)
	{
		int bytes_read;

		ff_assert(buffer->capacity > 0);
		ff_assert(buffer->size >= 0);
		ff_assert(buffer->start_pos >= 0);
		ff_assert(buffer->start_pos + buffer->size <= buffer->capacity);

		if (buffer->size == 0)
		{
			/* The buffer is empty. Try to read data from the underlying stream
			 * directly to the char_buf until the requested len is higher than
			 * buffer capacity. This allows to avoid superflous copying of data
			 * into buffer before copying it to the char_buf.
			 * If the requested len is less than the buffer capacity, then fill the buffer
			 * (read up to buffer->capacity bytes to the buffer). This allows to minimize
			 * the number of buffer->read_func() calls, because it is likely that subsequent calls
			 * to the ff_read_stream_buffer_read() will read data from the buffer.
			 */
			while (len >= buffer->capacity)
			{
				bytes_read = buffer->read_func(buffer->func_ctx, char_buf, len);
				if (bytes_read == -1)
				{
					goto end;
				}
				if (bytes_read == 0)
				{
					/* end of stream reached, but we didn't read requested len bytes of data,
					 * so treat this as an error.
					 */
					goto end;
				}
				ff_assert(bytes_read > 0);
				ff_assert(bytes_read <= len);
				char_buf += bytes_read;
				len -= bytes_read;
			}
			if (len == 0)
			{
				/* all requested data successfully read directly from the underlying stream */
				break;
			}

			bytes_read = buffer->read_func(buffer->func_ctx, buffer->buf, buffer->capacity);
			if (bytes_read == -1)
			{
				goto end;
			}
			if (bytes_read == 0)
			{
				/* end of stream reached, but we didn't read requested len bytes of data,
				 * so tread this as an error.
				 */
				goto end;
			}
			buffer->size = bytes_read;
			buffer->start_pos = 0;
		}
		ff_assert(buffer->size > 0);

		/* copy up to requested len bytes from the buffer into the char_buf */
		bytes_read = len > buffer->size ? buffer->size : len;
		ff_assert(bytes_read > 0);
		memcpy(char_buf, buffer->buf + buffer->start_pos, bytes_read);

		buffer->start_pos += bytes_read;
		buffer->size -= bytes_read;

		char_buf += bytes_read;
		len -= bytes_read;
	}
	result = FF_SUCCESS;

end:
	return result;
}
