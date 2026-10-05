#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>

#include "fifo_ops.h"


/*
 * Each parameter is implemented as a pair of callbacks:
 *   set - used by "echo VALUE > parameter"
 *   get - used by "cat parameter"
 *
 * This is intentional for operation-like parameters:
 * dequeue is a read operation with a side effect, while enqueue and
 * clear are write operations.
 */

static int param_set_enqueue(const char *val,
			     const struct kernel_param *kp)
{
	int value;
	int ret;

	if (kstrtoint(val, 10, &value))
		return -EINVAL;

	ret = fifo_enqueue(value);
	switch (ret) {
	case FIFO_OK:
		return 0;
	case FIFO_FULL:
		pr_warn("kernel_fifo: enqueue: FIFO is full\n");
		return -ENOSPC;
	case FIFO_INVALID:
		return -EINVAL;
	default:
		return -EIO;
	}
}

static int param_get_dequeue(char *buf,
			     const struct kernel_param *kp)
{
	int value = fifo_dequeue();

	if (value == FIFO_EMPTY)
		return sysfs_emit(buf, "error: FIFO is empty (%d)\n",
				  FIFO_EMPTY);
	if (value == FIFO_INVALID)
		return sysfs_emit(buf, "error: invalid FIFO (%d)\n",
				  FIFO_INVALID);

	return sysfs_emit(buf, "%d\n", value);
}

static int param_get_peek(char *buf,
			  const struct kernel_param *kp)
{
	int value = fifo_peek();

	if (value == FIFO_EMPTY)
		return sysfs_emit(buf, "error: FIFO is empty (%d)\n",
				  FIFO_EMPTY);
	if (value == FIFO_INVALID)
		return sysfs_emit(buf, "error: invalid FIFO (%d)\n",
				  FIFO_INVALID);

	return sysfs_emit(buf, "%d\n", value);
}

static int param_get_size(char *buf,
			  const struct kernel_param *kp)
{
	return sysfs_emit(buf, "%d\n", fifo_size());
}

static int param_get_available(char *buf,
			       const struct kernel_param *kp)
{
	return sysfs_emit(buf, "%d\n", fifo_available());
}

static int param_get_is_empty(char *buf,
			       const struct kernel_param *kp)
{
	return sysfs_emit(buf, "%d\n", fifo_is_empty());
}

static int param_get_is_full(char *buf,
			     const struct kernel_param *kp)
{
	return sysfs_emit(buf, "%d\n", fifo_is_full());
}

static int param_set_clear(const char *val,
			   const struct kernel_param *kp)
{
	int value;

	/*
	 * Any syntactically valid integer starts the clear operation.
	 * The actual value is deliberately ignored.
	 */
	if (kstrtoint(val, 10, &value))
		return -EINVAL;

	fifo_clear();
	return 0;
}

static const struct kernel_param_ops param_ops_enqueue = {
	.set = param_set_enqueue,
};

static const struct kernel_param_ops param_ops_dequeue = {
	.get = param_get_dequeue,
};

static const struct kernel_param_ops param_ops_peek = {
	.get = param_get_peek,
};

static const struct kernel_param_ops param_ops_size = {
	.get = param_get_size,
};

static const struct kernel_param_ops param_ops_available = {
	.get = param_get_available,
};

static const struct kernel_param_ops param_ops_is_empty = {
	.get = param_get_is_empty,
};

static const struct kernel_param_ops param_ops_is_full = {
	.get = param_get_is_full,
};

static const struct kernel_param_ops param_ops_clear = {
	.set = param_set_clear,
};

module_param_cb(enqueue, &param_ops_enqueue, NULL, 0200);
MODULE_PARM_DESC(enqueue, "Enqueue integer into FIFO");

module_param_cb(dequeue, &param_ops_dequeue, NULL, 0444);
MODULE_PARM_DESC(dequeue, "Dequeue and print first integer");

module_param_cb(peek, &param_ops_peek, NULL, 0444);
MODULE_PARM_DESC(peek, "Print first integer without removing it");

module_param_cb(size, &param_ops_size, NULL, 0444);
MODULE_PARM_DESC(size, "Current number of FIFO elements");

module_param_cb(available, &param_ops_available, NULL, 0444);
MODULE_PARM_DESC(available, "Number of free FIFO element slots");

module_param_cb(is_empty, &param_ops_is_empty, NULL, 0444);
MODULE_PARM_DESC(is_empty, "1 if FIFO is empty, 0 otherwise");

module_param_cb(is_full, &param_ops_is_full, NULL, 0444);
MODULE_PARM_DESC(is_full, "1 if FIFO is full, 0 otherwise");

module_param_cb(clear, &param_ops_clear, NULL, 0200);
MODULE_PARM_DESC(clear, "Clear FIFO; any integer value triggers the operation");
