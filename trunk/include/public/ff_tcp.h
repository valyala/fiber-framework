#ifndef FF_TCP_PUBLIC
#define FF_TCP_PUBLIC

#include "public/arch/ff_arch_net_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_tcp;

struct ff_tcp *ff_tcp_create();

void ff_tcp_delete(struct ff_tcp *tcp);

int ff_tcp_bind(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr, int is_listening);

int ff_tcp_connect(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr);

struct ff_tcp *ff_tcp_accept(struct ff_tcp *tcp, struct ff_arch_net_addr *remote_addr);

int ff_tcp_read(struct ff_tcp *tcp, void *buf, int len);

int ff_tcp_read_with_timeout(struct ff_tcp *tcp, void *buf, int len, int timeout);

int ff_tcp_write(struct ff_tcp *tcp, const void *buf, int len);

int ff_tcp_write_with_timeout(struct ff_tcp *tcp, const void *buf, int len, int timeout);

void ff_tcp_disconnect(struct ff_tcp *tcp);

#ifdef __cplusplus
}
#endif

#endif
