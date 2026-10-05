#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#include "stack_ops.h"

extern struct stack kernel_stack;

/*
 * Sysfs handlers may be called concurrently from different processes.
 * Protect the list and its size with a mutex.
 */
static DEFINE_MUTEX(stack_lock);

int stack_push(int value)
{
	struct stack_entry *entry;

	entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return STACK_NOMEM;

	entry->data = value;

	mutex_lock(&stack_lock);
	list_add(&entry->list, &kernel_stack.elements);
	kernel_stack.size++;
	mutex_unlock(&stack_lock);

	return STACK_OK;
}

int stack_pop(void)
{
	struct stack_entry *entry;
	int value;

	mutex_lock(&stack_lock);

	if (list_empty(&kernel_stack.elements)) {
		mutex_unlock(&stack_lock);
		return STACK_EMPTY;
	}

	entry = list_first_entry(&kernel_stack.elements,
				 struct stack_entry, list);
	list_del(&entry->list);
	kernel_stack.size--;
	value = entry->data;

	mutex_unlock(&stack_lock);

	kfree(entry);
	return value;
}

int stack_peek(void)
{
	struct stack_entry *entry;
	int value;

	mutex_lock(&stack_lock);

	if (list_empty(&kernel_stack.elements)) {
		mutex_unlock(&stack_lock);
		return STACK_EMPTY;
	}

	entry = list_first_entry(&kernel_stack.elements,
				 struct stack_entry, list);
	value = entry->data;

	mutex_unlock(&stack_lock);

	return value;
}

int stack_is_empty(void)
{
	int empty;

	mutex_lock(&stack_lock);
	empty = list_empty(&kernel_stack.elements) ? 1 : 0;
	mutex_unlock(&stack_lock);

	return empty;
}

int stack_size(void)
{
	int size;

	mutex_lock(&stack_lock);
	size = kernel_stack.size;
	mutex_unlock(&stack_lock);

	return size;
}

void stack_clear(void)
{
	struct stack_entry *entry, *tmp;

	mutex_lock(&stack_lock);

	list_for_each_entry_safe(entry, tmp, &kernel_stack.elements, list) {
		list_del(&entry->list);
		kfree(entry);
	}

	kernel_stack.size = 0;

	mutex_unlock(&stack_lock);
}
