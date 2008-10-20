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

/**
 * Creates a tcp.
 */
FF_API struct ff_tcp *ff_tcp_create();

/**
 * Deletes the given tcp.
 */
FF_API void ff_tcp_delete(struct ff_tcp *tcp);

/**
 * Binds the given tcp to the given addr.
 * If the type is FF_TCP_SERVER, then it also enables listening mode for the tcp
 * in order to be able to call ff_tcp_accept() on the given tcp.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_tcp_bind(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr, enum ff_tcp_type type);

/**
 * Connects the given tcp to the given addr.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_tcp_connect(struct ff_tcp *tcp, const struct ff_arch_net_addr *addr);

/**
 * Accepts tcp connection from remote peer.
 * Returns accepted connection on success and sets remote_addr to the address of the remote peer.
 * Returns NULL only if ff_tcp_disconnect() was called for the given tcp.
 */
FF_API struct ff_tcp *ff_tcp_accept(struct ff_tcp *tcp, struct ff_arch_net_addr *remote_addr);

/**
 * Reads exactly len bytes from the tcp into the buf.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_tcp_read(struct ff_tcp *tcp, void *buf, int len);

/**
 * Reads exactly len bytes from the tcp into the buf.
 * If the data cannot be read during the timeout milliseconds, then returns 0.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_tcp_read_with_timeout(struct ff_tcp *tcp, void *buf, int len, int timeout);

/**
 * Writes exactly len bytes from the buf into the tcp.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_tcp_write(struct ff_tcp *tcp, const void *buf, int len);

/**
 * Writes exactly len bytes from the buf into the tcp.
 * If the data cannot be written during the timeout milliseconds, then returns 0.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_tcp_write_with_timeout(struct ff_tcp *tcp, const void *buf, int len, int timeout);

/**
 * Flushes the tcp write buffer.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_tcp_flush(struct ff_tcp *tcp);

/**
 * Flushes the tcp write buffer.
 * If the buffer cannot be flushed during the timeout milliseconds, then returns 0.
 * Returns 1 on success, 0 on error.
 */
FF_API int ff_tcp_flush_with_timeout(struct ff_tcp *tcp, int timeout);

/**
 * Disconnects the tcp.
 * It unblocks blocked ff_tcp_accept(), ff_tcp_read*(), ff_tcp_write*() and ff_tcp_flush*() calls,
 * so they immediately return 0 (failure).
 * Subsequent calls to the ff_tcp_read*(), ff_tcp_write*() and ff_tcp_flush*() will return 0.
 */
FF_API void ff_tcp_disconnect(struct ff_tcp *tcp);

#ifdef __cplusplus
}
#endif

#endif
