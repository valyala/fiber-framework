#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_completion_port.h"
#include "ff_win_completion_port.h"
#include "private/ff_dictionary.h"

#define OVERLAPPED_DICTIONARY_ORDER 4

struct ff_arch_completion_port
{
	HANDLE handle;
	struct ff_dictionary *overlapped_dictionary;
};

struct ff_arch_completion_port *ff_arch_completion_port_create(int concurrency)
{
	struct ff_arch_completion_port *completion_port;
	
	completion_port = (struct ff_arch_completion_port *) ff_malloc(sizeof(*completion_port));
	completion_port->handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR) NULL, concurrency);
	ff_winapi_fatal_error_check(completion_port->handle != NULL, L"cannot create completion port");
	completion_port->overlapped_dictionary = ff_dictionary_create(OVERLAPPED_DICTIONARY_ORDER);
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
	BOOL result;
	
	result = GetQueuedCompletionStatus(
		completion_port->handle,
		&bytes_transferred,
		&key,
		&overlapped,
		INFINITE
	);

	if (overlapped != NULL)
	{
		enum ff_result result;

		result = ff_dictionary_get(completion_port->overlapped_dictionary, overlapped, data);
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
	ff_dictionary_put(completion_port->overlapped_dictionary, overlapped, data);
}

void ff_win_completion_port_deregister_overlapped_data(struct ff_arch_completion_port *completion_port, LPOVERLAPPED overlapped)
{
	void *data;
	enum ff_result result;

	result = ff_dictionary_remove_entry(completion_port->overlapped_dictionary, overlapped, &data);
	ff_assert(result == FF_SUCCESS);
}

void ff_win_completion_port_register_handle(struct ff_arch_completion_port *completion_port, HANDLE handle)
{
	HANDLE result_handle;
	ULONG_PTR key;

	key = (ULONG_PTR) NULL;
	result_handle = CreateIoCompletionPort(handle, completion_port->handle, key, 0);
	ff_winapi_fatal_error_check(result_handle == completion_port->handle, L"cannot assign handle to completion port");
}
