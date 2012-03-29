#include "private/ff_common.h"

#include "private/ff_udp.h"
#include "private/arch/ff_arch_udp.h"
#include "private/ff_core.h"

struct ff_udp
{
	struct ff_arch_udp *udp;
};

static void cancel_udp_operation(struct ff_fiber *fiber, void *ctx)
{
	struct ff_udp *udp;

	(void)fiber;
	udp = (struct ff_udp *) ctx;
	ff_udp_disconnect(udp);
}

struct ff_udp *ff_udp_create(enum ff_udp_type type)
{
	struct ff_udp *udp;
	int is_broadcast;

	is_broadcast = ((type == FF_UDP_BROADCAST) ? 1 : 0);
	udp = (struct ff_udp *) ff_malloc(sizeof(*udp));
	udp->udp = ff_arch_udp_create(is_broadcast);

	return udp;
}

void ff_udp_delete(struct ff_udp *udp)
{
	ff_arch_udp_delete(udp->udp);
	ff_free(udp);
}

enum ff_result ff_udp_bind(struct ff_udp *udp, const struct ff_arch_net_addr *addr)
{
	enum ff_result result;

	result = ff_arch_udp_bind(udp->udp, addr);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"cannot bind the udp=%p to the addr=%p. See previous messages for more info", udp, addr);
	}
	return result;
}

int ff_udp_read(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len)
{
	int bytes_read;

	ff_assert(len >= 0);

	bytes_read = ff_arch_udp_read(udp->udp, peer_addr, buf, len);
	if (bytes_read == -1)
	{
		ff_log_debug(L"error while reading data from the udp=%p into the buf=%p, len=%d, peer_addr=%p. See previous messages for more info", udp, buf, len, peer_addr);
	}
	return bytes_read;
}

int ff_udp_read_with_timeout(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	enum ff_result result;
	int bytes_read;

	ff_assert(len >= 0);
	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_udp_operation, udp);
	bytes_read = ff_udp_read(udp, peer_addr, buf, len);
	if (bytes_read == -1)
	{
		ff_log_debug(L"error while reading data from the udp=%p into the buf=%p, len=%d, peer_addr=%p using timeout=%d. See previous messages for more info",
			udp, buf, len, peer_addr, timeout);
	}
	result = ff_core_deregister_timeout_operation(timeout_operation_data);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"timeout=%d has been expired on read operation from the udp=%p into the buf=%p, len=%d, peer_addr=%p", timeout, udp, buf, len, peer_addr);
	}

	return bytes_read;
}

int ff_udp_write(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len)
{
	int bytes_written;

	ff_assert(len >= 0);

	bytes_written = ff_arch_udp_write(udp->udp, addr, buf, len);
	if (bytes_written == -1)
	{
		ff_log_debug(L"error while writing data to the udp=%p from the buf=%p, len=%d, addr=%p. See previous messages for more info", udp, buf, len, addr);
	}
	return bytes_written;
}

int ff_udp_write_with_timeout(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	enum ff_result result;
	int bytes_written;

	ff_assert(len >= 0);
	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_udp_operation, udp);
	bytes_written = ff_udp_write(udp, addr, buf, len);
	if (bytes_written == -1)
	{
		ff_log_debug(L"error while writing data to the udp=%p from the buf=%p, len=%d, addr=%p using timeout=%d. See prevsious messages for more info",
			udp, buf, len, addr, timeout);
	}
	result = ff_core_deregister_timeout_operation(timeout_operation_data);
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"timeout=%d has been expired on write operation to the udp=%p from the buf=%p, len=%d, addr=%p", timeout, udp, buf, len, addr);
	}

	return bytes_written;
}

void ff_udp_disconnect(struct ff_udp *udp)
{
	ff_arch_udp_disconnect(udp->udp);
}
