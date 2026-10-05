#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/kstrtox.h>

#include "stack_ops.h"
#include "kernel_stack.h"

static struct kobject *kernel_stack_kobj;

/*
 * /sys/kernel/kernel_stack/push
 */
static ssize_t push_store(struct kobject *kobj,
			  struct kobj_attribute *attr,
			  const char *buf, size_t count)
{
	int value;
	int ret;

	ret = kstrtoint(buf, 10, &value);
	if (ret)
		return STACK_INVALID;

	ret = stack_push(value);
	if (ret == STACK_NOMEM)
		return -ENOMEM;

	return count;
}

/*
 * /sys/kernel/kernel_stack/pop
 *
 * Reading this attribute performs the destructive pop operation.
 * A sysfs read can be repeated by userspace with a non-zero offset,
 * so return EOF after the first read.
 */
static ssize_t pop_show(struct kobject *kobj,
			 struct kobj_attribute *attr,
			 char *buf)
{
	int value;

	value = stack_pop();
	if (value == STACK_EMPTY)
		return scnprintf(buf, PAGE_SIZE, "STACK_EMPTY\n");

	return scnprintf(buf, PAGE_SIZE, "%d\n", value);
}

/*
 * /sys/kernel/kernel_stack/peek
 */
static ssize_t peek_show(struct kobject *kobj,
			  struct kobj_attribute *attr,
			  char *buf)
{
	int value;

	value = stack_peek();
	if (value == STACK_EMPTY)
		return scnprintf(buf, PAGE_SIZE, "STACK_EMPTY\n");

	return scnprintf(buf, PAGE_SIZE, "%d\n", value);
}

/*
 * /sys/kernel/kernel_stack/size
 */
static ssize_t size_show(struct kobject *kobj,
			 struct kobj_attribute *attr,
			 char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", stack_size());
}

/*
 * /sys/kernel/kernel_stack/is_empty
 */
static ssize_t is_empty_show(struct kobject *kobj,
			     struct kobj_attribute *attr,
			     char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", stack_is_empty());
}

/*
 * /sys/kernel/kernel_stack/clear
 */
static ssize_t clear_store(struct kobject *kobj,
			   struct kobj_attribute *attr,
			   const char *buf, size_t count)
{
	stack_clear();
	return count;
}

/*
 * __ATTR_* is used here rather than DEVICE_ATTR_* because the requested
 * path is directly below /sys/kernel. A kobject is the natural sysfs
 * object for this location; DEVICE_ATTR_* is intended for struct device.
 */
static struct kobj_attribute push_attribute =
	__ATTR_WO(push);

static struct kobj_attribute pop_attribute =
	__ATTR_RO(pop);

static struct kobj_attribute peek_attribute =
	__ATTR_RO(peek);

static struct kobj_attribute size_attribute =
	__ATTR_RO(size);

static struct kobj_attribute is_empty_attribute =
	__ATTR_RO(is_empty);

static struct kobj_attribute clear_attribute =
	__ATTR_WO(clear);

static struct attribute *kernel_stack_attrs[] = {
	&push_attribute.attr,
	&pop_attribute.attr,
	&peek_attribute.attr,
	&size_attribute.attr,
	&is_empty_attribute.attr,
	&clear_attribute.attr,
	NULL,
};

static const struct attribute_group kernel_stack_attr_group = {
	.attrs = kernel_stack_attrs,
};

int kernel_stack_sysfs_init(void)
{
	int ret;

	kernel_stack_kobj = kobject_create_and_add("kernel_stack", kernel_kobj);
	if (!kernel_stack_kobj)
		return -ENOMEM;

	ret = sysfs_create_group(kernel_stack_kobj, &kernel_stack_attr_group);
	if (ret) {
		kobject_put(kernel_stack_kobj);
		kernel_stack_kobj = NULL;
		return ret;
	}

	return 0;
}

void kernel_stack_sysfs_exit(void)
{
	if (!kernel_stack_kobj)
		return;

	sysfs_remove_group(kernel_stack_kobj, &kernel_stack_attr_group);
	kobject_put(kernel_stack_kobj);
	kernel_stack_kobj = NULL;
}
