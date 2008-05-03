#include "private/ff_common.h"

#include "private/ff_stack.h"

struct stack_entry
{
	struct stack_entry *next;
	void *data;
};

struct ff_stack
{
	struct stack_entry *top;
};

struct ff_stack *ff_stack_create()
{
	struct ff_stack *stack = (struct ff_stack *) ff_malloc(sizeof(*stack));
	stack->top = NULL;

	return stack;
}

void ff_stack_delete(struct ff_stack *stack)
{
	ff_assert(stack->top == NULL);

	ff_free(stack);
}

void ff_stack_push(struct ff_stack *stack, void *data)
{
	struct stack_entry *entry = (struct stack_entry *) ff_malloc(sizeof(*entry));
	entry->next = stack->top;
	entry->data = data;
	stack->top = entry;
}

int ff_stack_is_empty(struct ff_stack *stack)
{
	int is_empty = stack->top == NULL ? 1 : 0;
	return is_empty;
}

void *ff_stack_top(struct ff_stack *stack)
{
	ff_assert(stack->top != NULL);

	return stack->top->data;
}

void ff_stack_pop(struct ff_stack *stack)
{
	struct stack_entry *entry;

	ff_assert(stack->top != NULL);

	entry = stack->top;
	stack->top = entry->next;
	ff_free(entry);
}

int ff_stack_remove_entry(struct ff_stack *stack, void *data)
{
	int is_removed = 0;
	struct stack_entry **entry_ptr = &stack->top;
	struct stack_entry *entry = stack->top;

	while (entry != NULL)
	{
		if (entry->data == data)
		{
			*entry_ptr = entry->next;
			ff_free(entry);
			is_removed = 1;
			break;
		}
		entry_ptr = &entry->next;
		entry = entry->next;
	}

	return is_removed;
}
