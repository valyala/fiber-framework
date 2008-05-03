#ifndef FF_FIBER_PUBLIC
#define FF_FIBER_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * the opaque fiber structure
 */
struct ff_fiber;

/**
 * @public
 * declaration of fiber function, which should be passed to the ff_fiber_create()
 */
typedef void (*ff_fiber_func)(void *ctx);

/**
 * @public
 * creates the new fiber, which will execute the given fiber_func
 * after calling the ff_fiber_start().
 * stack_size is the size of the stack for the given fiber. If it is set to 0,
 * then the stack size will be set automatically.
 * This function always returns correct result.
 */
struct ff_fiber *ff_fiber_create(ff_fiber_func fiber_func, int stack_size);

/**
 * @public
 * deletes the given fiber
 */
void ff_fiber_delete(struct ff_fiber *fiber);

/**
 * @public
 * schedules the given fiber for execution and passes the ctx to the fiber_func,
 * which has been set in the ff_fiber_create()
 */
void ff_fiber_start(struct ff_fiber *fiber, void *ctx);

/**
 * @public
 * waits while the given fiber will be finished,
 * i.e. it will exit the fiber_func, which was passed to the ff_fiber_create().
 * This function returns immediately in two cases:
 *   1) if the fiber was created but wasn't yet started by calling the ff_fiber_start();
 *   2) if the fiber was already finished by leaving the fiber_func.
 */
void ff_fiber_join(struct ff_fiber *fiber);

#ifdef __cplusplus
}
#endif

#endif
