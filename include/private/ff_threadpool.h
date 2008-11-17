#ifndef FF_THREADPOOL_PRIVATE_H
#define FF_THREADPOOL_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

struct ff_threadpool;

struct ff_threadpool *ff_threadpool_create(int max_threads_cnt);

void ff_threadpool_delete(struct ff_threadpool *threadpool);

typedef void (*ff_threadpool_func)(void *ctx);

void ff_threadpool_execute_async(struct ff_threadpool *threadpool, ff_threadpool_func func, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
