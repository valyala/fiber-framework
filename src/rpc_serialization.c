#include "private/ff_common.h"

struct rpc_stream;

int rpc_stream_read(struct rpc_stream *stream, void *buf, int len);
int rpc_stream_write(struct rpc_stream *stream, const void *buf, int len);
void rpc_stream_close(struct rpc_stream *stream);

#define BUF_SIZE 0x10000

int rpc_stream_copy(struct rpc_stream *dst_stream, struct rpc_stream *src_stream, int len)
{
	int bytes_copied;
	uint8_t *buf;

	ff_assert(len >= 0);

	buf = (uint8_t *) ff_malloc(BUF_SIZE);
	bytes_copied = 0;
	while (len > 0)
	{
		int bytes_read;
		int bytes_written;
		int bytes_to_read;

		bytes_to_read = len > BUF_SIZE ? BUF_SIZE : len;

		bytes_read = rpc_stream_read(src_stream, buf, bytes_to_read);
		if (bytes_read != bytes_to_read)
		{
			bytes_copied = -1;
			break;
		}

		bytes_written = rpc_stream_write(dst_stream, buf, bytes_read);
		if (bytes_written != bytes_read)
		{
			bytes_copied = -1;
			break;
		}

		bytes_copied += bytes_written;
		len -= bytes_written;
	}

	ff_free(buf);

	return bytes_copied;
}

#define MAX_STRING_SIZE 0x10000

#define BITS_PER_OCTET 7
#define MAX_UINT_N_OCTETS(n) (((n) + BITS_PER_OCTET - 1) / BITS_PER_OCTET)
#define MAX_UINT32_OCTETS MAX_UINT_N_OCTETS(32)
#define MAX_UINT64_OCTETS MAX_UINT_N_OCTETS(64)
#define OCTET_MASK ((1 << BITS_PER_OCTET) - 1)

int uint32_serialize(uint32_t data, struct rpc_stream *stream)
{
	int bytes_written;
	int len;
	uint8_t buf[MAX_UINT32_OCTETS];

	len = 0;
	do
	{
		uint8_t octet;
		
		ff_assert(len < MAX_UINT32_OCTETS);
		octet = (uint8_t) (data & OCTET_MASK);
		data >>= BITS_PER_OCTET;
		octet |= (uint8_t) ((data != 0) << BITS_PER_OCTET);
		buf[len] = octet;
		len++;
	}
	while (data != 0);

	bytes_written = rpc_stream_write(stream, buf, len);
	if (bytes_written != len)
	{
		bytes_written = -1;
	}

	return bytes_written;
}

int uint64_serialize(uint64_t data, struct rpc_stream *stream)
{
	int bytes_written;
	int len;
	uint8_t buf[MAX_UINT64_OCTETS];

	len = 0;
	do
	{
		uint8_t octet;
		
		ff_assert(len < MAX_UINT64_OCTETS);
		octet = (uint8_t) (data & OCTET_MASK);
		data >>= BITS_PER_OCTET;
		octet |= (uint8_t) ((data != 0) << BITS_PER_OCTET);
		buf[len] = octet;
		len++;
	}
	while (data != 0);

	bytes_written = rpc_stream_write(stream, buf, len);
	if (bytes_written != len)
	{
		bytes_written = -1;
	}

	return bytes_written;
}

int uint32_unserialize(uint32_t *data, struct rpc_stream *stream)
{
	int bytes_read;
	uint32_t u_data;
	uint8_t octet;

	u_data = 0;
	bytes_read = 0;
	do
	{
		int len;

		if (bytes_read >= MAX_UINT32_OCTETS)
		{
			bytes_read = -1;
			goto end;
		}
		len = rpc_stream_read(stream, &octet, 1);
		if (len != 1)
		{
			bytes_read = -1;
			goto end;
		}
		bytes_read++;
		u_data <<= BITS_PER_OCTET;
		u_data |= (octet & OCTET_MASK);
	}
	while ((octet >> BITS_PER_OCTET) != 0);

	*data = u_data;

end:
	return bytes_read;
}

int uint64_unserialize(uint64_t *data, struct rpc_stream *stream)
{
	int bytes_read;
	uint64_t u_data;
	uint8_t octet;

	u_data = 0;
	bytes_read = 0;
	do
	{
		int len;

		if (bytes_read >= MAX_UINT64_OCTETS)
		{
			bytes_read = -1;
			break;
		}
		len = rpc_stream_read(stream, &octet, 1);
		if (len != 1)
		{
			bytes_read = -1;
			break;
		}
		bytes_read++;
		u_data <<= BITS_PER_OCTET;
		u_data |= (octet & OCTET_MASK);
	}
	while ((octet >> BITS_PER_OCTET) != 0);

	return bytes_read;
}

int int32_serialize(int32_t data, struct rpc_stream *stream)
{
	int bytes_written;
	uint32_t u_data;

	u_data = (data << 1) ^ (data >> 31);
	bytes_written = uint32_serialize(u_data, stream);

	return bytes_written;
}

int int64_serialize(int64_t data, struct rpc_stream *stream)
{
	int bytes_written;
	uint64_t u_data;

	u_data = (data << 1) ^ (data >> 63);
	bytes_written = uint64_serialize(u_data, stream);

	return bytes_written;
}

int int32_unserialize(int32_t *data, struct rpc_stream *stream)
{
	int bytes_read;
	uint32_t u_data;

	bytes_read = uint32_unserialize(&u_data, stream);
	u_data = (u_data >> 1) ^ (-(int32_t)(u_data & 0x01));
	*data = (int32_t) u_data;

	return bytes_read;
}

int int64_unserialize(int64_t *data, struct rpc_stream *stream)
{
	int bytes_read;
	uint64_t u_data;

	bytes_read = uint64_unserialize(&u_data, stream);
	u_data = (u_data >> 1) ^ (-(int64_t)(u_data & 0x01));
	*data = (int64_t) u_data;

	return bytes_read;
}

int string_serialize(const wchar_t *str, int str_len, struct rpc_stream *stream)
{
	int bytes_written = -1;
	int i;

	ff_assert(str_len >= 0);

	if (str_len > MAX_STRING_SIZE)
	{
		goto end;
	}

	bytes_written = uint32_serialize((uint32_t) str_len, stream);
	if (bytes_written == -1)
	{
		goto end;
	}
	for (i = 0; i < str_len; i++)
	{
		uint32_t ch;
		int len;

		ch = (uint32_t) str[i];
		len = uint32_serialize(ch, stream);
		if (len == -1)
		{
			bytes_written = -1;
			goto end;
		}

		bytes_written += len;
	}

end:
	return bytes_written;
}

int string_unserialize(wchar_t **str, int *str_len, struct rpc_stream *stream)
{
	int bytes_read;
	uint32_t i;
	uint32_t u_str_len;
	uint32_t str_size;
	wchar_t *new_str;

	bytes_read = uint32_unserialize(&u_str_len, stream);
	if (bytes_read == -1)
	{
		goto end;
	}
	if (u_str_len > MAX_STRING_SIZE)
	{
		bytes_read = -1;
		goto end;
	}
	*str_len = (int) u_str_len;

	/* integer overflow is impossible here, because MAX_STRING_SIZE and, consequently,
	 * u_str_len is guaranteed to be less than MAX_INT / sizeof(wchar_t)
	 */
	str_size = sizeof(wchar_t) * u_str_len;
	new_str = (wchar_t *) ff_malloc(str_size);

	for (i = 0; i < u_str_len; i++)
	{
		uint32_t ch;
		int len;

		len = uint32_unserialize(&ch, stream);
		if (len == -1)
		{
			bytes_read = -1;
			ff_free(new_str);
			goto end;
		}
		new_str[i] = (wchar_t) ch;

		bytes_read += len;
	}

	*str = new_str;

end:
	return bytes_read;
}

enum blob_open_stream_mode
{
	BLOB_READ,
	BLOB_WRITE,
};

struct blob *blob_create(int blob_size);
void blob_delete(struct blob *blob);
int blob_get_size(const struct blob *blob);
struct rpc_stream *blob_open_stream(const struct blob *blob, enum blob_open_stream_mode mode);

int blob_serialize(const struct blob *blob, struct rpc_stream *stream)
{
	int bytes_written;
	int bytes_copied;
	int blob_len;
	struct rpc_stream *blob_stream;

	blob_len = blob_get_size(blob);
	if (blob_len < 0)
	{
		goto end;
	}
	bytes_written = uint32_serialize(blob_len, stream);
	if (bytes_written == -1 || bytes_written + blob_len <= 0)
	{
		bytes_written = -1;
		goto end;
	}

	blob_stream = blob_open_stream(blob, BLOB_READ);
	if (blob_stream == NULL)
	{
		bytes_written = -1;
		goto end;
	}
	bytes_copied = rpc_stream_copy(stream, blob_stream, blob_len);
	rpc_stream_close(blob_stream);
	if (bytes_copied != blob_len)
	{
		bytes_written = -1;
		goto end;
	}
	bytes_written += bytes_copied;

end:
	return bytes_written;
}

int blob_unserialize(struct blob **blob, struct rpc_stream *stream)
{
	int bytes_read;
	int bytes_copied;
	int blob_len;
	struct blob *new_blob;
	struct rpc_stream *blob_stream;

	bytes_read = uint32_unserialize((uint32_t *) &blob_len, stream);
	if (bytes_read == -1 || blob_len < 0)
	{
		bytes_read = -1;
		goto end;
	}
	new_blob = blob_create(blob_len);
	blob_stream = blob_open_stream(new_blob, BLOB_WRITE);
	if (blob_stream == NULL)
	{
		bytes_read = -1;
		blob_delete(new_blob);
		goto end;
	}
	bytes_copied = rpc_stream_copy(blob_stream, stream, blob_len);
	rpc_stream_close(blob_stream);
	if (bytes_copied != blob_len)
	{
		bytes_read = -1;
		blob_delete(new_blob);
		goto end;
	}
	bytes_read += bytes_copied;
	*blob = new_blob;

end:
	return bytes_read;
}
