#include "ff_win_stdafx.h"

#include "private/arch/ff_arch_completion_port.h"
#include "ff_win_completion_port.h"
#include "ff_win_error_check.h"
#include "private/ff_dictionary.h"

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
	completion_port->overlapped_dictionary = ff_dictionary_create();
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

void *ff_arch_completion_port_get(struct ff_arch_completion_port *completion_port)
{
	DWORD bytes_transferred;
	void *data;
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

	data = (void *) key;
	if (data == NULL)
	{
		if (overlapped != NULL)
		{
			int is_found;
			
			is_found = ff_dictionary_get(completion_port->overlapped_dictionary, overlapped, &data);
			ff_assert(is_found);
		}
	}

	return data;
}

void ff_arch_completion_port_put(struct ff_arch_completion_port *completion_port, void *data)
{
	ULONG_PTR key;
	BOOL result;

	key = (ULONG_PTR) data;
	result = PostQueuedCompletionStatus(completion_port->handle, 0, key, NULL);
	ff_assert(result != FALSE);
}

void ff_win_completion_port_register_overlapped_data(struct ff_arch_completion_port *completion_port, LPOVERLAPPED overlapped, void *data)
{
	ff_dictionary_put(completion_port->overlapped_dictionary, overlapped, data);
}

void ff_win_completion_port_deregister_overlapped_data(struct ff_arch_completion_port *completion_port, LPOVERLAPPED overlapped)
{
	void *data;
	int is_removed;

	is_removed = ff_dictionary_remove_entry(completion_port->overlapped_dictionary, overlapped, &data);
	ff_assert(is_removed);
}

void ff_win_completion_port_register_handle(struct ff_arch_completion_port *completion_port, HANDLE handle)
{
	HANDLE result_handle;
	ULONG_PTR key;

	key = (ULONG_PTR) NULL;
	result_handle = CreateIoCompletionPort(handle, completion_port->handle, key, 0);
	ff_winapi_fatal_error_check(result_handle == completion_port->handle, L"cannot assign handle to completion port");
}
