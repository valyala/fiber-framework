#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_completion_port.h"
#include "ff_win_completion_port.h"
#include "private/ff_dictionary.h"
#include "private/ff_hash.h"

#define OVERLAPPED_DICTIONARY_ORDER 8

#define OVERLAPPED_HASH_START_VALUE 0

struct ff_arch_completion_port
{
	HANDLE handle;
	struct ff_dictionary *overlapped_dictionary;
};

static uint32_t dictionary_get_overlapped_hash(const void *key)
{
	LPOVERLAPPED overlapped;
	uint32_t hash_value;
	int key_size;

	overlapped = (LPOVERLAPPED) key;
	key_size = sizeof(LPOVERLAPPED) / sizeof(uint32_t);
	ff_assert(key_size * sizeof(uint32_t) == sizeof(LPOVERLAPPED));
	/* it is ok that the calculated hash value will be different for low-endian and big-endian 64-bit architectures,
	 * because it is used only on local machine and doesn't passed outside.
	 */
	hash_value = ff_hash_uint32(OVERLAPPED_HASH_START_VALUE, (uint32_t *) &overlapped, key_size);
	return hash_value;
}

static int dictionary_is_equal_overlapped(const void *key1, const void *key2)
{
	int is_equal;

	is_equal = ((LPOVERLAPPED)key1 == (LPOVERLAPPED)key2);
	return is_equal;
}

struct ff_arch_completion_port *ff_arch_completion_port_create(int concurrency)
{
	struct ff_arch_completion_port *completion_port;
	
	completion_port = (struct ff_arch_completion_port *) ff_malloc(sizeof(*completion_port));
	completion_port->handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR) NULL, concurrency);
	ff_winapi_fatal_error_check(completion_port->handle != NULL, L"cannot create completion port");
	completion_port->overlapped_dictionary = ff_dictionary_create(OVERLAPPED_DICTIONARY_ORDER, dictionary_get_overlapped_hash, dictionary_is_equal_overlapped);
	return completion_port;
}

void ff_arch_completion_port_delete(struct ff_arch_completion_port *completion_port)
{
	BOOL result;

	ff_dictionary_delete(completion_port->overlapped_dictionary);
	result = CloseHandle(completion_port->handle);
	ff_assert(result != FALSE);
	ff_free(completion_port);
}

void ff_arch_completion_port_get(struct ff_arch_completion_port *completion_port, const void **data)
{
	DWORD bytes_transferred;
	ULONG_PTR key;
	LPOVERLAPPED overlapped;
	BOOL rv;
	
	rv = GetQueuedCompletionStatus(
		completion_port->handle,
		&bytes_transferred,
		&key,
		&overlapped,
		INFINITE
	);
	if (rv == FALSE)
	{
		DWORD last_error;

		last_error = GetLastError();
		ff_log_debug(L"GetQueuedCompletionStatus() failed on key=%p, overlapped=%p. GetLastError()=%lu", key, overlapped, last_error);
	}

	if (overlapped != NULL)
	{
		enum ff_result result;

		result = ff_dictionary_get_entry(completion_port->overlapped_dictionary, overlapped, data);
		ff_assert(result == FF_SUCCESS);
	}
	else
	{
		*data = (const void *) key;
	}
}

void ff_arch_completion_port_put(struct ff_arch_completion_port *completion_port, const void *data)
{
	ULONG_PTR key;
	BOOL result;

	key = (ULONG_PTR) data;
	result = PostQueuedCompletionStatus(completion_port->handle, 0, key, NULL);
	ff_assert(result != FALSE);
}

void ff_win_completion_port_register_overlapped_data(struct ff_arch_completion_port *completion_port, LPOVERLAPPED overlapped, const void *data)
{
	enum ff_result result;

	result = ff_dictionary_add_entry(completion_port->overlapped_dictionary, overlapped, data);
	ff_assert(result == FF_SUCCESS);
}

void ff_win_completion_port_deregister_overlapped_data(struct ff_arch_completion_port *completion_port, LPOVERLAPPED overlapped)
{
	LPOVERLAPPED overlapped_key;
	void *data;
	enum ff_result result;

	result = ff_dictionary_remove_entry(completion_port->overlapped_dictionary, overlapped, (const void **) &overlapped_key, &data);
	ff_assert(result == FF_SUCCESS);
	ff_assert(overlapped == overlapped_key);
}

void ff_win_completion_port_register_handle(struct ff_arch_completion_port *completion_port, HANDLE handle)
{
	HANDLE result_handle;
	ULONG_PTR key;

	key = (ULONG_PTR) NULL;
	result_handle = CreateIoCompletionPort(handle, completion_port->handle, key, 0);
	ff_winapi_fatal_error_check(result_handle == completion_port->handle, L"cannot assign handle to completion port");
}
