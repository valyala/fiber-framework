#include "private/ff_common.h"

#include "private/arch/ff_arch_net_addr.h"
#include "private/ff_core.h"
#include "ff_linux_net_addr.h"
#include "ff_linux_misc.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ADDR_TO_STRING_BUF_SIZE 22

struct threadpool_addr_resolve_data
{
	struct ff_arch_net_addr *addr;
	const char *host;
	int port;
	int is_success;
};

static void threadpool_addr_resolve_func(void *ctx)
{
	struct threadpool_addr_resolve_data *data;
	int rv;
	struct addrinfo hint;
	struct addrinfo *addr_info_ptr;

	data = (struct threadpool_addr_resolve_data *) ctx;
	data->is_success = 0;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = PF_INET;

	rv = getaddrinfo(data->host, NULL, &hint, &addr_info_ptr);
	if (rv == 0)
	{
		ff_assert(addr_info_ptr != NULL);
		ff_assert(addr_info_ptr->ai_addrlen == sizeof(data->addr->addr));
		memcpy(&data->addr->addr, addr_info_ptr->ai_addr, sizeof(data->addr->addr));
		freeaddrinfo(addr_info_ptr);
		data->is_success = 1;
		data->addr->addr.sin_port = htons((uint16_t) data->port);
	}
}

struct ff_arch_net_addr *ff_arch_net_addr_create()
{
	struct ff_arch_net_addr *addr;

	addr = (struct ff_arch_net_addr *) ff_malloc(sizeof(*addr));

	return addr;
}

void ff_arch_net_addr_delete(struct ff_arch_net_addr *addr)
{
	ff_free(addr);
}

int ff_arch_net_addr_resolve(struct ff_arch_net_addr *addr, const wchar_t *host, int port)
{
	char *mb_host;
	struct threadpool_addr_resolve_data data;

	ff_assert(port >= 0);
	ff_assert(port < 0x10000);

	mb_host = ff_linux_misc_wide_to_multibyte_string(host);
	data.addr = addr;
	data.host = mb_host;
	data.port = port;
	data.is_success = 0;
	ff_core_threadpool_execute(threadpool_addr_resolve_func, &data);
	ff_free(mb_host);

	return data.is_success;
}

void ff_arch_net_addr_get_broadcast_addr(const struct ff_arch_net_addr *addr, const struct ff_arch_net_addr *net_mask, struct ff_arch_net_addr *broadcast_addr)
{
	struct sockaddr_in tmp_addr;

	memcpy(&tmp_addr, &addr->addr, sizeof(tmp_addr));
	tmp_addr.sin_addr.s_addr |= ~(net_mask->addr.sin_addr.s_addr);
	memcpy(&broadcast_addr->addr, &tmp_addr, sizeof(tmp_addr));
}

int ff_arch_net_addr_is_equal(const struct ff_arch_net_addr *addr1, const struct ff_arch_net_addr *addr2)
{
	int is_equal;
	
	is_equal = (memcmp(&addr1->addr, &addr2->addr, sizeof(addr1->addr)) == 0);
	return is_equal;
}

const wchar_t *ff_arch_net_addr_to_string(const struct ff_arch_net_addr *addr)
{
	char *str;
	wchar_t *buf;
	int len;
	uint16_t port;

	str = inet_ntoa(addr->addr.sin_addr);
	ff_assert(str != NULL);
	port = ntohs(addr->addr.sin_port);
	buf = (wchar_t *) ff_calloc(ADDR_TO_STRING_BUF_SIZE, sizeof(buf[0]));
	len = swprintf(buf, ADDR_TO_STRING_BUF_SIZE, L"%s:%hu", str, port);
	ff_assert(len > 0);

	return buf;
}

void ff_arch_net_addr_delete_string(const wchar_t *str)
{
	ff_free((void *)str);
}

