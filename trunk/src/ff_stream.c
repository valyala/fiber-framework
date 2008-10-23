#include "private/ff_common.h"

#include "private/ff_stream.h"
#include "private/ff_hash.h"
#include "private/ff_log.h"

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
	return result;
}

enum ff_result ff_stream_write(struct ff_stream *stream, const void *buf, int len)
{
	enum ff_result result;

	ff_assert(len >= 0);

	result = stream->vtable->write(stream, buf, len);
	return result;
}

enum ff_result ff_stream_flush(struct ff_stream *stream)
{
	enum ff_result result;

	result = stream->vtable->flush(stream);
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
			ff_log_warning(L"error when copying streams: cannot read from src_stream");
			goto end;
		}
		result = ff_stream_write(dst_stream, buf, chunk_size);
		if (result != FF_SUCCESS)
		{
			ff_log_warning(L"error when copying streams: cannot write to the dst_stream");
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
			ff_log_warning(L"error when calculating hash: cannot read from the stream");
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
