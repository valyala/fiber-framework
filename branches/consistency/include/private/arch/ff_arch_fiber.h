#ifndef FF_ARCH_FIBER_PRIVATE
#define FF_ARCH_FIBER_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * opaque platform-specific structure of fiber
 */
struct ff_arch_fiber;

/**
 * @public
 * type definition for the arch_fiber_func.
 */
typedef void (*ff_arch_fiber_func)(void *ctx);

/**
 * @public
 * Converts the thread to fibers
 */
struct ff_arch_fiber *ff_arch_fiber_initialize();

/**
 * @public
 * Converts fiber to thread
 */
void ff_arch_fiber_shutdown(struct ff_arch_fiber *fiber);

/**
 * @public
 * creates the platform-specific fiber, which will execute the arch_fiber_func.
 * ctx will be passed as parameter to the arch_fiber_func.
 * stack_size is the size of stack for the fiber. If it is 0, then stack_size will be determined automatically.
 * Always returns correct result.
 */
struct ff_arch_fiber *ff_arch_fiber_create(ff_arch_fiber_func arch_fiber_func, void *ctx, int stack_size);

/**
 * @public
 * deletes the platform-specific fiber
 */
void ff_arch_fiber_delete(struct ff_arch_fiber *fiber);

/**
 * @public
 * Switches to the given fiber
 */
void ff_arch_fiber_switch(struct ff_arch_fiber *fiber);

#ifdef __cplusplus
}
#endif

#endif
