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
 * sleeps for the given interval milliseconds
 */
FF_API void ff_core_sleep(int interval);


typedef void (*ff_core_threadpool_func)(void *ctx);

/**
 * @public
 * Executes the func in the threadpool
 */
FF_API void ff_core_threadpool_execute(ff_core_threadpool_func func, void *ctx);

typedef void (*ff_core_fiberpool_func)(void *ctx);

/**
 * @public
 * Schedules the func for execution in the fiberpool
 */
FF_API void ff_core_fiberpool_execute_async(ff_core_fiberpool_func func, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
