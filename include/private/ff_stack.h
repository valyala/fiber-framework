#ifndef FF_STACK_PRIVATE
#define FF_STACK_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_stack;

struct ff_stack *ff_stack_create();

void ff_stack_delete(struct ff_stack *stack);

void ff_stack_push(struct ff_stack *stack, const void *data);

int ff_stack_is_empty(struct ff_stack *stack);

void ff_stack_top(struct ff_stack *stack, const void **data);

void ff_stack_pop(struct ff_stack *stack);

enum ff_result ff_stack_remove_entry(struct ff_stack *stack, const void *data);

#ifdef __cplusplus
}
#endif

#endif
