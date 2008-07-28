#ifndef FF_ARCH_COMPLETION_PORT_PRIVATE
#define FF_ARCH_COMPLETION_PORT_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_completion_port;

struct ff_arch_completion_port *ff_arch_completion_port_create(int concurrency);

void ff_arch_completion_port_delete(struct ff_arch_completion_port *completion_port);

void ff_arch_completion_port_get(struct ff_arch_completion_port *completion_port, const void **data);

void ff_arch_completion_port_put(struct ff_arch_completion_port *completion_port, const void *data);

#ifdef __cplusplus
}
#endif

#endif
