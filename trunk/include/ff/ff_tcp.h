#ifndef FF_TCP_PUBLIC
#define FF_TCP_PUBLIC

#include "ff/ff_common.h"
#include "ff/arch/ff_arch_net_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_tcp;

enum ff_tcp_type
{
	FF_TCP_SERVER,
	FF_TCP_CLIENT
};

FF_API struct ff_tcp *ff_tcp_create();

FF_API void ff_tcp_delete(struct ff_tcp *tcp);

FF_API int ff_tcp_bind(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr, enum ff_tcp_type type);

FF_API int ff_tcp_connect(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr);

FF_API struct ff_tcp *ff_tcp_accept(struct ff_tcp *tcp, struct ff_arch_net_addr *remote_addr);

FF_API int ff_tcp_read(struct ff_tcp *tcp, void *buf, int len);

FF_API int ff_tcp_read_with_timeout(struct ff_tcp *tcp, void *buf, int len, int timeout);

FF_API int ff_tcp_write(struct ff_tcp *tcp, const void *buf, int len);

FF_API int ff_tcp_write_with_timeout(struct ff_tcp *tcp, const void *buf, int len, int timeout);

FF_API int ff_tcp_flush(struct ff_tcp *tcp);

FF_API int ff_tcp_flush_with_timeout(struct ff_tcp *tcp, int timeout);

FF_API void ff_tcp_disconnect(struct ff_tcp *tcp);

#ifdef __cplusplus
}
#endif

#endif
