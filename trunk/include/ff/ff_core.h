#ifndef FF_CORE_PUBLIC
#define FF_CORE_PUBLIC

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * Initializes the fiber framework
 */
FF_API void ff_core_initialize();

/**
 * @public
 * Shutdowns the fiber framework
 */
FF_API void ff_core_shutdown();

/**
 * @public
 * sleeps the current fiber for the given interval milliseconds
 */
FF_API void ff_core_sleep(int interval);


typedef void (*ff_core_threadpool_func)(void *ctx);

/**
 * @public
 * Synchronously executes the func in the threadpool
 */
FF_API void ff_core_threadpool_execute(ff_core_threadpool_func func, void *ctx);

typedef void (*ff_core_fiberpool_func)(void *ctx);

/**
 * @public
 * Schedules the func for execution in the fiberpool
 */
FF_API void ff_core_fiberpool_execute_async(ff_core_fiberpool_func func, void *ctx);

/**
 * @public
 * Schedules the func for execution in the fiberpool after the given interval in milliseconds.
 * This function is more effective than the following construction:
 *   static void async_func(void *ctx) { ff_core_sleep(interval); func(ctx); }
 *   ff_core_fiberpool_execute_async(async_func, ctx);
 */
FF_API void ff_core_fiberpool_execute_deferred(ff_core_fiberpool_func func, void *ctx, int interval);

#ifdef __cplusplus
}
#endif

#endif
