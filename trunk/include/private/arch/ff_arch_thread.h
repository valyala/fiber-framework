#ifndef FF_ARCH_THREAD_PRIVATE
#define FF_ARCH_THREAD_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_arch_thread;

typedef void (*ff_arch_thread_func)(void *ctx);

struct ff_arch_thread *ff_arch_thread_create(ff_arch_thread_func func, int stack_size);

void ff_arch_thread_delete(struct ff_arch_thread *thread);

void ff_arch_thread_start(struct ff_arch_thread *thread, void *ctx);

void ff_arch_thread_join(struct ff_arch_thread *thread);

#ifdef __cplusplus
}
#endif

#endif
