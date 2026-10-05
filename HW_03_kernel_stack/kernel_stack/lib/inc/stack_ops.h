#ifndef STACK_OPS_H
#define STACK_OPS_H

#include "stack.h"

int stack_push(int value);
int stack_pop(void);
int stack_peek(void);
int stack_is_empty(void);
int stack_size(void);
void stack_clear(void);

#endif /* STACK_OPS_H */
