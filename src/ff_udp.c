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

	udp = (struct ff_udp *) ctx;
	ff_udp_disconnect(udp);
}

struct ff_udp *ff_udp_create(int is_broadcast)
{
	struct ff_udp *udp;

	udp = (struct ff_udp *) ff_malloc(sizeof(*udp));
	udp->udp = ff_arch_udp_create(is_broadcast);

	return udp;
}

void ff_udp_delete(struct ff_udp *udp)
{
	ff_arch_udp_delete(udp->udp);
	ff_free(udp);
}

int ff_udp_bind(struct ff_udp *udp, const struct ff_arch_net_addr *addr)
{
	int is_success;

	is_success = ff_arch_udp_bind(udp->udp, addr);
	return is_success;
}

int ff_udp_read(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len)
{
	int bytes_read;

	bytes_read = ff_arch_udp_read(udp->udp, peer_addr, buf, len);
	return bytes_read;
}

int ff_udp_read_with_timeout(struct ff_udp *udp, struct ff_arch_net_addr *peer_addr, void *buf, int len, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	int bytes_read;

	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_udp_operation, udp);
	bytes_read = ff_udp_read(udp, peer_addr, buf, len);
	ff_core_deregister_timeout_operation(timeout_operation_data);

	return bytes_read;
}

int ff_udp_write(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len)
{
	int bytes_written;

	bytes_written = ff_arch_udp_write(udp->udp, addr, buf, len);
	return bytes_written;
}

int ff_udp_write_with_timeout(struct ff_udp *udp, const struct ff_arch_net_addr *addr, const void *buf, int len, int timeout)
{
	struct ff_core_timeout_operation_data *timeout_operation_data;
	int bytes_written;

	ff_assert(timeout > 0);

	timeout_operation_data = ff_core_register_timeout_operation(timeout, cancel_udp_operation, udp);
	bytes_written = ff_udp_write(udp, addr, buf, len);
	ff_core_deregister_timeout_operation(timeout_operation_data);

	return bytes_written;
}

void ff_udp_disconnect(struct ff_udp *udp)
{
	ff_arch_udp_disconnect(udp->udp);
}
