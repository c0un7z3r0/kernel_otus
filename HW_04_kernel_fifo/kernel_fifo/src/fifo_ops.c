#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/slab.h>

#include "fifo_ops.h"


int fifo_init(int size)
{
	unsigned int bytes;
	int ret;

	if (size <= 0)
		return FIFO_INVALID;

	mutex_init(&fifo_dev.lock);
	fifo_dev.max_size = 0;

	bytes = (unsigned int)size * sizeof(struct fifo_entry);

	ret = kfifo_alloc(&fifo_dev.queue, bytes, GFP_KERNEL);
	if (ret)
		return FIFO_NOMEM;

	/*
	 * kfifo uses a power-of-two sized buffer. Therefore its real
	 * capacity in entries can be larger than the requested size.
	 * Restrict the public FIFO to exactly 'size' entries.
	 */
	fifo_dev.max_size = size;

	return FIFO_OK;
}

void fifo_destroy(void)
{
	mutex_lock(&fifo_dev.lock);

	kfifo_free(&fifo_dev.queue);
	fifo_dev.max_size = 0;

	mutex_unlock(&fifo_dev.lock);
}

int fifo_enqueue(int value)
{
	struct fifo_entry entry;
	unsigned int copied;

	if (fifo_dev.max_size <= 0)
		return FIFO_INVALID;

	mutex_lock(&fifo_dev.lock);

	if (kfifo_len(&fifo_dev.queue) / sizeof(struct fifo_entry) >=
	    (unsigned int)fifo_dev.max_size) {
		mutex_unlock(&fifo_dev.lock);
		return FIFO_FULL;
	}

	entry.data = value;
	copied = kfifo_in(&fifo_dev.queue, &entry, sizeof(entry));

	mutex_unlock(&fifo_dev.lock);

	return copied == sizeof(entry) ? FIFO_OK : FIFO_FULL;
}

int fifo_dequeue(void)
{
	struct fifo_entry entry;
	unsigned int copied;

	if (fifo_dev.max_size <= 0)
		return FIFO_INVALID;

	mutex_lock(&fifo_dev.lock);

	if (kfifo_is_empty(&fifo_dev.queue)) {
		mutex_unlock(&fifo_dev.lock);
		return FIFO_EMPTY;
	}

	copied = kfifo_out(&fifo_dev.queue, &entry, sizeof(entry));

	mutex_unlock(&fifo_dev.lock);

	if (copied != sizeof(entry))
		return FIFO_EMPTY;

	return entry.data;
}

int fifo_peek(void)
{
	struct fifo_entry entry;
	unsigned int copied;

	if (fifo_dev.max_size <= 0)
		return FIFO_INVALID;

	mutex_lock(&fifo_dev.lock);

	if (kfifo_is_empty(&fifo_dev.queue)) {
		mutex_unlock(&fifo_dev.lock);
		return FIFO_EMPTY;
	}

	copied = kfifo_out_peek(&fifo_dev.queue, &entry, sizeof(entry));

	mutex_unlock(&fifo_dev.lock);

	if (copied != sizeof(entry))
		return FIFO_EMPTY;

	return entry.data;
}

int fifo_is_empty(void)
{
	int result;

	if (fifo_dev.max_size <= 0)
		return 1;

	mutex_lock(&fifo_dev.lock);
	result = kfifo_is_empty(&fifo_dev.queue);
	mutex_unlock(&fifo_dev.lock);

	return result;
}

int fifo_is_full(void)
{
	int result;

	if (fifo_dev.max_size <= 0)
		return 0;

	mutex_lock(&fifo_dev.lock);
	result = (kfifo_len(&fifo_dev.queue) / sizeof(struct fifo_entry) >=
		  (unsigned int)fifo_dev.max_size);
	mutex_unlock(&fifo_dev.lock);

	return result;
}

int fifo_size(void)
{
	int result;

	if (fifo_dev.max_size <= 0)
		return 0;

	mutex_lock(&fifo_dev.lock);
	result = (int)(kfifo_len(&fifo_dev.queue) /
		       sizeof(struct fifo_entry));
	mutex_unlock(&fifo_dev.lock);

	return result;
}

int fifo_available(void)
{
	int used;
	int result;

	if (fifo_dev.max_size <= 0)
		return 0;

	mutex_lock(&fifo_dev.lock);
	used = (int)(kfifo_len(&fifo_dev.queue) /
		     sizeof(struct fifo_entry));
	result = fifo_dev.max_size - used;
	mutex_unlock(&fifo_dev.lock);

	return result;
}

void fifo_clear(void)
{
	if (fifo_dev.max_size <= 0)
		return;

	mutex_lock(&fifo_dev.lock);
	kfifo_reset(&fifo_dev.queue);
	mutex_unlock(&fifo_dev.lock);
}

int fifo_capacity(void)
{
	return fifo_dev.max_size;
}
