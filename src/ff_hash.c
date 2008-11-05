#include "private/ff_common.h"

#include "private/ff_hash.h"

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

static uint32_t get_big_endian_uint16_hash(uint32_t start_value, const uint16_t *buf, int buf_size)
{
	int uint32_chunks_cnt;
	uint32_t hash;
	int tail_size;

	ff_assert(buf_size >= 0);

	uint32_chunks_cnt = buf_size >> 1;
	hash = ff_hash_uint32(start_value, (const uint32_t *) buf, uint32_chunks_cnt);
	tail_size = buf_size & 0x01;
	if (tail_size == 1)
	{
		uint32_t tail_uint32;
		const uint16_t *tail;

		tail = buf + (uint32_chunks_cnt << 1);
		tail_uint32 = (uint32_t) tail[0];
		hash = ff_hash_uint32(hash, &tail_uint32, 1);
	}
	return hash;
}

/**
 * Calculates Bob Jenkins' hash for the given buf with the given buf_size size and the given start_value.
 * The function is based on the research http://www.burtleburtle.net/bob/hash/doobs.html
 * It is probably the fastest hash function for word elements
 * and gives distribution close to uniform for typical input.
 * See http://burtleburtle.net/bob/c/lookup3.c for reference implementation.
 */
uint32_t ff_hash_uint32(uint32_t start_value, const uint32_t *buf, int buf_size)
{
	uint32_t aligned_array[3];
	uint32_t a, b, c;

	ff_assert(buf_size >= 0);

	a = b = c = 0xdeadbeef + (((uint32_t) buf_size) << 2) + start_value;
	while (buf_size > 3)
	{
		memcpy(aligned_array, buf, 3 * sizeof(aligned_array[0]));
		a += aligned_array[0];
		b += aligned_array[1];
		c += aligned_array[2];
		mix(a, b, c);
		buf_size -= 3;
		buf += 3;
	}

	memcpy(aligned_array, buf, buf_size * sizeof(aligned_array[0]));
	switch (buf_size)
	{
		case 3 : c += aligned_array[2];
		case 2 : b += aligned_array[1];
		case 1 : a += aligned_array[0];
			final(a, b, c);
		case 0:
			break;
	}

	return c;
}

uint32_t ff_hash_uint16(uint32_t start_value, const uint16_t *buf, int buf_size)
{
	static const uint32_t test = 1;
	static const char *p_test = (const char *) &test;
	uint32_t hash_value;
	
	/* this test for endiannes must be removed by optimizing compiler */
	if (p_test[0] == 1)
	{
		hash_value = get_big_endian_uint16_hash(start_value, buf, buf_size);
	}
	else if (p_test[3] == 1)
	{
		uint16_t *tmp_buf;
		int i;

		tmp_buf = (uint16_t *) ff_calloc(buf_size, sizeof(tmp_buf[0]));
		for (i = 0; i < buf_size; i++)
		{
			uint16_t tmp;

			tmp = buf[i];
			tmp = (tmp << 8) | (tmp >> 8);
			tmp_buf[i] = tmp;
		}
		hash_value = get_big_endian_uint16_hash(start_value, buf, buf_size);
		ff_free(tmp_buf);
	}
	else
	{
		/* unexpected endiannes has been detected ;) */
		ff_assert(0);
	}
	return hash_value;
}

uint32_t ff_hash_uint8(uint32_t start_value, const uint8_t *buf, int buf_size)
{
	int uint32_chunks_cnt;
	uint32_t hash;
	int tail_size;

	ff_assert(buf_size >= 0);

	uint32_chunks_cnt = buf_size >> 2;
	hash = ff_hash_uint32(start_value, (const uint32_t *) buf, uint32_chunks_cnt);
	tail_size = buf_size & 0x03;
	if (tail_size > 0)
	{
		uint32_t tail_uint32;
		const uint8_t *tail;

		tail = buf + (uint32_chunks_cnt << 2);
		switch (tail_size)
		{
			case 3:
				tail_uint32 = (((uint32_t) tail[0]) << 16) | (((uint32_t) tail[1]) << 8) | ((uint32_t) tail[2]);
				break;
			case 2:
				tail_uint32 = (((uint32_t) tail[0]) << 8) | ((uint32_t) tail[1]);
				break;
			case 1:
				tail_uint32 = (uint32_t) tail[0];
				break;
		}
		hash = ff_hash_uint32(hash, &tail_uint32, 1);
	}
	return hash;
}
