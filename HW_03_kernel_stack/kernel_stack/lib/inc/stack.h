#ifndef STACK_H
#define STACK_H

#include <linux/list.h>

/* Stack operation status codes. */
#define STACK_OK       0
#define STACK_EMPTY   -1
#define STACK_NOMEM   -2
#define STACK_INVALID -3

struct stack {
	struct list_head elements;
	int size;
};

struct stack_entry {
	struct list_head list;
	int data;
};

void stack_init(void);

#endif /* STACK_H */
