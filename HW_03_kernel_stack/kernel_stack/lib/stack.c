#include <linux/list.h>
#include "stack.h"

/*
 * The actual stack object is kept in this translation unit.
 * All access to it is performed through stack_ops.c.
 */
struct stack kernel_stack;

void stack_init(void)
{
	INIT_LIST_HEAD(&kernel_stack.elements);
	kernel_stack.size = 0;
}
