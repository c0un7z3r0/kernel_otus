#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "kernel_stack.h"
#include "stack_ops.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("c0un7z3r0");
MODULE_DESCRIPTION("Linux kernel module implementing a stack using kernel linked lists and sysfs");
MODULE_VERSION("1.0");

static int __init kernel_stack_init(void)
{
	int ret;

	stack_init();

	ret = kernel_stack_sysfs_init();
	if (ret) {
		stack_clear();
		pr_err("kernel_stack: failed to initialize sysfs: %d\n", ret);
		return ret;
	}

	pr_info("kernel_stack: module loaded\n");
	pr_info("kernel_stack: sysfs interface at /sys/kernel/kernel_stack/\n");

	return 0;
}

static void __exit kernel_stack_exit(void)
{
	kernel_stack_sysfs_exit();
	stack_clear();

	pr_info("kernel_stack: module unloaded\n");
}

module_init(kernel_stack_init);
module_exit(kernel_stack_exit);
