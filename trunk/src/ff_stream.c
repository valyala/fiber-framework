#include "private/ff_common.h"

#include "private/ff_stream.h"
#include "private/ff_hash.h"

#define BUF_SIZE 0x10000

struct ff_stream
{
	const struct ff_stream_vtable *vtable;
	void *ctx;
};

struct ff_stream *ff_stream_create(const struct ff_stream_vtable *vtable, void *ctx)
{
	struct ff_stream *stream;

	stream = (struct ff_stream *) ff_malloc(sizeof(*stream));
	stream->vtable = vtable;
	stream->ctx = ctx;
	return stream;
}

void ff_stream_delete(struct ff_stream *stream)
{
	stream->vtable->delete(stream);
	ff_free(stream);
}

void *ff_stream_get_ctx(struct ff_stream *stream)
{
	return stream->ctx;
}

enum ff_result ff_stream_read(struct ff_stream *stream, void *buf, int len)
{
	enum ff_result result;

	ff_assert(len >= 0);

	result = stream->vtable->read(stream, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot read data from the stream=%p to the buf=%p, len=%d. See previous messages for more info", stream, buf, len);
	}
	return result;
}

enum ff_result ff_stream_write(struct ff_stream *stream, const void *buf, int len)
{
	enum ff_result result;

	ff_assert(len >= 0);

	result = stream->vtable->write(stream, buf, len);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot write data to the stream=%p from the buf=%p, len=%d. See previous messages for more info", stream, buf, len);
	}
	return result;
}

enum ff_result ff_stream_flush(struct ff_stream *stream)
{
	enum ff_result result;

	result = stream->vtable->flush(stream);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot flush the stream=%p. See previous messages for more info", stream);
	}
	return result;
}

void ff_stream_disconnect(struct ff_stream *stream)
{
	stream->vtable->disconnect(stream);
}

enum ff_result ff_stream_copy(struct ff_stream *src_stream, struct ff_stream *dst_stream, int len)
{
	uint8_t *buf;
	enum ff_result result = FF_FAILURE;

	ff_assert(len >= 0);

	buf = (uint8_t *) ff_calloc(BUF_SIZE, sizeof(buf[0]));
	while (len > 0)
	{
		int chunk_size;

		chunk_size = len > BUF_SIZE ? BUF_SIZE : len;
		result = ff_stream_read(src_stream, buf, chunk_size);
		if (result != FF_SUCCESS)
		{
			ff_log_debug(L"cannot read from src_stream=%p to buf=%p, chunk_size=%d. See previous messages for more info", src_stream, buf, chunk_size);
			goto end;
		}
		result = ff_stream_write(dst_stream, buf, chunk_size);
		if (result != FF_SUCCESS)
		{
			ff_log_debug(L"cannot write to the dst_stream=%p from buf=%p, chunk_size=%d. See previous messages for more info", dst_stream, buf, chunk_size);
			goto end;
		}
		len -= chunk_size;
	}
	result = FF_SUCCESS;

end:
	ff_free(buf);
	return result;
}

enum ff_result ff_stream_get_hash(struct ff_stream *stream, int len, uint32_t start_value, uint32_t *hash_value)
{
	uint8_t *buf;
	uint32_t hash;
	enum ff_result result = FF_FAILURE;

	ff_assert(len >= 0);

	hash = start_value;
	buf = (uint8_t *) ff_calloc(BUF_SIZE, sizeof(buf[0]));
	while (len > 0)
	{
		int chunk_size;

		chunk_size = len > BUF_SIZE ? BUF_SIZE : len;
		result = ff_stream_read(stream, buf, chunk_size);
		if (result != FF_SUCCESS)
		{
			ff_log_debug(L"cannot read from the stream=%p to buf=%p, chunk_size=%d. See previous messages for more info", stream, buf, chunk_size);
			goto end;
		}
		hash = ff_hash_uint8(hash, buf, chunk_size);
		len -= chunk_size;
	}
	*hash_value = hash;
	result = FF_SUCCESS;

end:
	ff_free(buf);
	return result;
}
