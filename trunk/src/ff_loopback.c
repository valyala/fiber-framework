#include "private/ff_common.h"
#include "private/ff_loopback.h"
#include "private/ff_event.h"

struct ff_loopback
{
	struct ff_event *read_event;
	struct ff_event *write_event;
	char *buffer;
	char *read_ptr;
	char *write_ptr;
	int buffer_size;
	int is_disconnected;
};

struct ff_loopback *ff_loopback_create(int buffer_size)
{
	struct ff_loopback *loopback;

	ff_assert(buffer_size > 1);

	loopback = (struct ff_loopback *) ff_malloc(sizeof(*loopback));
	loopback->read_event = ff_event_create(FF_EVENT_AUTO);
	loopback->write_event = ff_event_create(FF_EVENT_AUTO);
	loopback->buffer = (char *) ff_malloc(buffer_size);
	loopback->read_ptr = loopback->buffer;
	loopback->write_ptr = loopback->buffer;
	loopback->buffer_size = buffer_size;
	loopback->is_disconnected = 0;

	return loopback;
}

void ff_loopback_delete(struct ff_loopback *loopback)
{
	ff_assert(loopback != NULL);

	ff_free(loopback->buffer);
	ff_event_delete(loopback->write_event);
	ff_event_delete(loopback->read_event);
	ff_free(loopback);
}

enum ff_result ff_loopback_read(struct ff_loopback *loopback, void *buf, int len)
{
	char *char_buf;
	enum ff_result result = FF_FAILURE;

	ff_assert(loopback != NULL);
	ff_assert(buf != NULL);
	ff_assert(len >= 0);

	char_buf = buf;
	while (len > 0)
	{
		int bytes_read;
		int bytes_left;

		if (loopback->read_ptr <= loopback->write_ptr)
		{
			bytes_left = loopback->write_ptr - loopback->read_ptr;
			if (bytes_left == 0)
			{
				if (loopback->is_disconnected)
				{
					ff_log_debug(L"the loopback=%p is disconnected, so it cannot read the rest of len=%d bytes to the buf=%p", loopback, len, buf);
					goto end;
				}
				ff_event_wait(loopback->read_event);
				continue;
			}
			else if (bytes_left == loopback->buffer_size - 1)
			{
				ff_event_set(loopback->write_event);
			}
		}
		else
		{
			if (loopback->read_ptr - loopback->write_ptr == 1)
			{
				ff_event_set(loopback->write_event);
			}
			bytes_left = loopback->buffer_size - (loopback->read_ptr - loopback->buffer);
		}
		ff_assert(bytes_left > 0);

		bytes_read = len > bytes_left ? bytes_left : len;
		memcpy(char_buf, loopback->read_ptr, bytes_read);

		char_buf += bytes_read;
		loopback->read_ptr += bytes_read;
		len -= bytes_read;

		if (loopback->read_ptr - loopback->buffer == loopback->buffer_size)
		{
			loopback->read_ptr = loopback->buffer;
		}
	}
	result = FF_SUCCESS;

end:
	return result;
}

enum ff_result ff_loopback_write(struct ff_loopback *loopback, const void *buf, int len)
{
	const char *char_buf;
	enum ff_result result = FF_FAILURE;

	ff_assert(loopback != NULL);
	ff_assert(buf != NULL);
	ff_assert(len >= 0);

	char_buf = buf;
	while (len > 0)
	{
		int bytes_written;
		int bytes_left;

		if (loopback->is_disconnected)
		{
			ff_log_debug(L"the loopback=%p is disconnected, so it cannot write the rest of len=%d bytes to the buf=%p", loopback, len, buf);
			goto end;
		}

		if (loopback->write_ptr < loopback->read_ptr)
		{
			bytes_left = (loopback->read_ptr - loopback->write_ptr) - 1;
			if (bytes_left == 0)
			{
				ff_event_wait(loopback->write_event);
				continue;
			}
		}
		else
		{
			if (loopback->write_ptr == loopback->read_ptr)
			{
				ff_event_set(loopback->read_event);
			}
			else if (loopback->write_ptr - loopback->read_ptr == loopback->buffer_size - 1)
			{
				ff_event_wait(loopback->write_event);
				continue;
			}
			bytes_left = loopback->buffer_size - (loopback->write_ptr - loopback->buffer);
			if (bytes_left == loopback->buffer_size)
			{
				bytes_left--;
			}
		}
		ff_assert(bytes_left > 0);

		bytes_written = len > bytes_left ? bytes_left : len;
		memcpy(loopback->write_ptr, char_buf, bytes_written);

		char_buf += bytes_written;
		loopback->write_ptr += bytes_written;
		len -= bytes_written;

		if (loopback->write_ptr - loopback->buffer == loopback->buffer_size)
		{
			loopback->write_ptr = loopback->buffer;
		}
	}
	result = FF_SUCCESS;

end:
	return result;
}

void ff_loopback_disconnect(struct ff_loopback *loopback)
{
	ff_assert(loopback != NULL);

	if (!loopback->is_disconnected)
	{
		loopback->is_disconnected = 1;
		ff_event_set(loopback->read_event);
		ff_event_set(loopback->write_event);
	}
}
