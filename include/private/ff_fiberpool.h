#ifndef FF_FIBERPOOL_PRIVATE
#define FF_FIBERPOOL_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_fiberpool;

struct ff_fiberpool *ff_fiberpool_create(int max_fibers_cnt);

void ff_fiberpool_delete(struct ff_fiberpool *fiberpool);

typedef void (*ff_fiberpool_func)(void *ctx);

void ff_fiberpool_execute_async(struct ff_fiberpool *fiberpool, ff_fiberpool_func func, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
