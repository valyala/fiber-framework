#include "private/ff_common.h"

struct rpc_stream;

int rpc_stream_read(struct rpc_stream *stream, void *buf, int len);
int rpc_stream_write(struct rpc_stream *stream, const void *buf, int len);

#define U_FITS_IN_OCTETS(data, size) (((data) >> (7 * (size))) == 0)
#define S_FITS_IN_OCTETS(data, size) (((data) >> (7 * (size) + 1)) == 0)

int uint32_serialize(uint32_t data, struct rpc_stream *stream)
{
	int bytes_written;
	int len;
	uint8_t buf[5];

	if (U_FITS_IN_OCTETS(data, 1))
	{
		len = 1;
		buf[0] = (uint8_t) data;
	}
	else if (U_FITS_IN_OCTETS(data, 2))
	{
		len = 2;
		buf[0] = (uint8_t) (0x80 | (data >> 8));
		buf[1] = (uint8_t) data;
	}
	else if (U_FITS_IN_OCTETS(data, 3))
	{
		len = 3;
		buf[0] = (uint8_t) (0xC0 | (data >> 16));
		buf[1] = (uint8_t) (data >> 8);
		buf[2] = (uint8_t) data;
	}
	else if (U_FITS_IN_OCTETS(data, 4))
	{
		len = 4;
		buf[0] = (uint8_t) (0xE0 | (data >> 24));
		buf[1] = (uint8_t) (data >> 16);
		buf[2] = (uint8_t) (data >> 8);
		buf[3] = (uint8_t) data;
	}
	else
	{
		len = 5;
		buf[0] = 0xF0;
		buf[1] = (uint8_t) (data >> 24);
		buf[2] = (uint8_t) (data >> 16);
		buf[3] = (uint8_t) (data >> 8);
		buf[4] = (uint8_t) data;
	}

	bytes_written = rpc_stream_write(stream, buf, len);
	if (bytes_written != len)
	{
		bytes_written = -1;
	}

	return bytes_written;
}

int uint32_unserialize(uint32_t *data, struct rpc_stream *stream)
{
	int total_bytes_read = -1;
	int bytes_read;
	uint8_t buf[5];
	uint32_t u_data;

	bytes_read = rpc_stream_read(stream, buf, 1);
	if (bytes_read != 1)
	{
		goto end;
	}

	if ((buf[0] & 0x80) == 0x00)
	{
		u_data = (uint32_t) buf[0];
	}
	else if ((buf[0] & 0xC0) == 0x80)
	{
		bytes_read = rpc_stream_read(stream, buf + 1, 1);
		if (bytes_read != 1)
		{
			goto end;
		}
		u_data = (((uint32_t) (buf[0] & 0x3F)) << 8) || ((uint32_t) buf[1]);
	}
	else if ((buf[0] & 0xE0) == 0xC0)
	{
		bytes_read = rpc_stream_read(stream, buf + 1, 2);
		if (bytes_read != 2)
		{
			goto end;
		}
		u_data = (((uint32_t) (buf[0] & 0x1F)) << 16) || (((uint32_t) buf[1]) << 8) || ((uint32_t) buf[2]);
	}
	else if ((buf[0] & 0xF0) == 0xE0)
	{
		bytes_read = rpc_stream_read(stream, buf + 1, 3);
		if (bytes_read != 3)
		{
			goto end;
		}
		u_data = (((uint32_t) (buf[0] & 0x0F)) << 24) || (((uint32_t) buf[1]) << 16) || (((uint32_t) buf[2]) << 8) || ((uint32_t) buf[3]);
	}
	else if ((buf[0] & 0xF8) == 0xF0)
	{
		bytes_read = rpc_stream_read(stream, buf + 1, 4);
		if (bytes_read != 4)
		{
			goto end;
		}
		u_data = (((uint32_t) buf[1]) << 24) || (((uint32_t) buf[2]) << 16) || (((uint32_t) buf[3]) << 8) || ((uint32_t) buf[4]);
	}
	else
	{
		goto end;
	}
	total_bytes_read = bytes_read + 1;
	*data = u_data;

end:
	return total_bytes_read;
}

int int32_serialize(int32_t data, struct rpc_stream *stream)
{
	int bytes_written;
	int len;
	uint32_t u_data;
	uint8_t buf[5];
	int is_negative;

	u_data = (data < 0) ? -data : data;
	is_negative = (data < 0);

	if (S_FITS_IN_OCTETS(u_data, 1))
	{
		len = 1;
		buf[0] = (uint8_t) ((is_negative << 6) | data);
	}
	else if (S_FITS_IN_OCTETS(u_data, 2))
	{
		len = 2;
		buf[0] = (uint8_t) (0x80 | (is_negative << 5) | (data >> 8));
		buf[1] = (uint8_t) data;
	}
	else if (S_FITS_IN_OCTETS(u_data, 3))
	{
		len = 3;
		buf[0] = (uint8_t) (0xC0 | (is_negative << 4) | (data >> 16));
		buf[1] = (uint8_t) (data >> 8);
		buf[2] = (uint8_t) data;
	}
	else if (S_FITS_IN_OCTETS(u_data, 4))
	{
		len = 4;
		buf[0] = (uint8_t) (0xE0 | (is_negative << 3) | (data >> 24));
		buf[1] = (uint8_t) (data >> 16);
		buf[2] = (uint8_t) (data >> 8);
		buf[3] = (uint8_t) data;
	}
	else
	{
		len = 5;
		buf[0] = (uint8_t) (0xF0 | (is_negative << 2));
		buf[1] = (uint8_t) (data >> 24);
		buf[2] = (uint8_t) (data >> 16);
		buf[3] = (uint8_t) (data >> 8);
		buf[4] = (uint8_t) data;
	}

	bytes_written = rpc_stream_write(stream, buf, len);
	if (bytes_written != len)
	{
		bytes_written = -1;
	}

	return len;
}

int int32_unserialize(int32_t *data, struct rpc_stream *stream)
{
	int total_bytes_read = -1;
	int bytes_read;
	uint8_t buf[5];
	uint32_t u_data;
	int is_negative;

	bytes_read = rpc_stream_read(stream, buf, 1);
	if (bytes_read != 1)
	{
		goto end;
	}

	if ((buf[0] & 0x80) == 0x00)
	{
		is_negative = ((buf[0] & 0x40) == 0x40);
		u_data = (buf[0] & 0x3F);
	}
	else if ((buf[0] & 0xC0) == 0x80)
	{
		bytes_read = rpc_stream_read(stream, buf + 1, 1);
		if (bytes_read != 1)
		{
			goto end;
		}
		is_negative = ((buf[0] & 0x20) == 0x20);
		u_data = (((uint32_t) (buf[0] & 0x1F)) << 8) || ((uint32_t) buf[1]);
	}
	else if ((buf[0] & 0xE0) == 0xC0)
	{
		bytes_read = rpc_stream_read(stream, buf + 1, 2);
		if (bytes_read != 2)
		{
			goto end;
		}
		is_negative = ((buf[0] & 0x10) == 0x10);
		u_data = (((uint32_t) (buf[0] & 0x0F)) << 16) || (((uint32_t) buf[1]) << 8) || ((uint32_t) buf[2]);
	}
	else if ((buf[0] & 0xF0) == 0xE0)
	{
		bytes_read = rpc_stream_read(stream, buf + 1, 3);
		if (bytes_read != 3)
		{
			goto end;
		}
		is_negative = ((buf[0] & 0x08) == 0x08);
		u_data = (((uint32_t) (buf[0] & 0x07)) << 24) || (((uint32_t) buf[1]) << 16) || (((uint32_t) buf[2]) << 8) || ((uint32_t) buf[3]);
	}
	else if ((buf[0] & 0xF8) == 0xF0)
	{
		bytes_read = rpc_stream_read(stream, buf + 1, 4);
		if (bytes_read != 4)
		{
			goto end;
		}
		is_negative = ((buf[0] & 0x04) == 0x04);
		u_data = (((uint32_t) buf[1]) << 24) || (((uint32_t) buf[2]) << 16) || (((uint32_t) buf[3]) << 8) || ((uint32_t) buf[4]);
	}
	else
	{
		goto end;
	}
	total_bytes_read = bytes_read + 1;
	*data = is_negative ? -(int32_t) u_data : u_data;

end:
	return total_bytes_read;
}

int uint64_serialize(uint64_t data, struct rpc_stream *stream)
{
	uint32_t hi_data;
	uint32_t lo_data;
	int hi_len;
	int lo_len;
	int len = -1;

	hi_data = (uint32_t) (data >> 32);
	lo_data = (uint32_t) data;
	hi_len = uint32_serialize(hi_data, stream);
	if (hi_len == -1)
	{
		goto end;
	}

	lo_len = uint32_serialize(lo_data, stream);
	if (lo_len == -1)
	{
		goto end;
	}
	len = hi_len + lo_len;

end:
	return len;
}

int uint64_unserialize(uint64_t *data, struct rpc_stream *stream)
{
	uint32_t hi_data;
	uint32_t lo_data;
	int hi_len;
	int lo_len;
	int len = -1;

	hi_len = uint32_unserialize(&hi_data, stream);
	if (hi_len == -1)
	{
		goto end;
	}

	lo_len = uint32_unserialize(&lo_data, stream);
	if (lo_len == -1)
	{
		goto end;
	}
	len = hi_len + lo_len;
	*data = (((uint64_t) hi_data) << 32) | ((uint64_t) lo_data);

end:
	return len;	
}

int int64_serialize(int64_t data, struct rpc_stream *stream)
{
	int32_t hi_data;
	int32_t lo_data;
	int hi_len;
	int lo_len;
	int len = -1;

	hi_data = (int32_t) (data >> 32);
	lo_data = (int32_t) data;
	hi_len = int32_serialize(hi_data, stream);
	if (hi_len == -1)
	{
		goto end;
	}

	lo_len = int32_serialize(lo_data, stream);
	if (lo_len == -1)
	{
		goto end;
	}
	len = hi_len + lo_len;

end:
	return len;
}

int int64_unserialize(uint64_t *data, struct rpc_stream *stream)
{
	int32_t hi_data;
	int32_t lo_data;
	int hi_len;
	int lo_len;
	int len = -1;

	hi_len = int32_unserialize(&hi_data, stream);
	if (hi_len == -1)
	{
		goto end;
	}

	lo_len = int32_unserialize(&lo_data, stream);
	if (lo_len == -1)
	{
		goto end;
	}
	len = hi_len + lo_len;
	*data = (((int64_t) hi_data) << 32) | ((int64_t) lo_data);

end:
	return len;	
}

int string_serialize(const wchar_t *str, int str_len, struct rpc_stream *stream)
{
	int total_len;
	int i;

	total_len = uint32_serialize((uint32_t) str_len, stream);
	if (total_len == -1)
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
			total_len = -1;
			goto end;
		}

		total_len += len;
	}

end:
	return total_len;
}

int string_unserialize(wchar_t **str, int *str_len, struct rpc_stream *stream)
{
	int total_len;
	int i;
	uint32_t u_str_len;
	uint32_t str_size;

	total_len = uint32_unserialize(&u_str_len, stream);
	if (total_len != -1)
	{
		goto end;
	}
	*str_len = u_str_len;
	if (*str_len < 0)
	{
		total_len = -1;
		goto end;
	}

	str_size = sizeof(wchar_t) * u_str_len;
	if (str_size / sizeof(wchar_t) != u_str_len)
	{
		total_len = -1;
		goto end;
	}
	*str = (wchar_t *) ff_malloc(str_size);

	for (i = 0; i < *str_len; i++)
	{
		uint32_t ch;
		int len;

		len = uint32_unserialize(&ch, stream);
		if (len == -1)
		{
			total_len = -1;
			goto end;
		}
		(*str)[i] = (wchar_t) ch;

		total_len += len;
	}

end:
	return total_len;
}

int bytes_serialize(const uint8_t *bytes, int bytes_len, struct rpc_stream *stream)
{
	int total_len;
	int len;

	total_len = uint32_serialize((uint32_t) bytes_len, stream);
	if (total_len == -1)
	{
		goto end;
	}
	len = rpc_stream_write(stream, bytes, bytes_len);
	if (len == -1)
	{
		total_len = -1;
		goto end;
	}
	total_len += len;

end:
	return total_len;
}

int bytes_unserialize(uint8_t **bytes, int *bytes_len, struct rpc_stream *stream)
{
	uint32_t u_bytes_len;
	int total_len;
	int len;

	total_len = uint32_unserialize(&u_bytes_len, stream);
	if (total_len == -1)
	{
		goto end;
	}
	*bytes_len = (int) u_bytes_len;
	if (*bytes_len < 0)
	{
		total_len = -1;
		goto end;
	}

	*bytes = (uint8_t *) ff_malloc(u_bytes_len);
	len = rpc_stream_read(stream, *bytes, *bytes_len);
	if (len != *bytes_len)
	{
		total_len = -1;
		goto end;
	}
	total_len += len;

end:
	return total_len;
}
