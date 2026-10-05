#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "fifo_ops.h"


struct fifo_device fifo_dev;

static int __init kernel_fifo_init(void)
{
	int ret;

	ret = fifo_init(DEFAULT_FIFO_SIZE);
	if (ret != FIFO_OK) {
		pr_err("kernel_fifo: failed to initialize FIFO: %d\n", ret);
		return -ENOMEM;
	}

	pr_info("kernel_fifo: loaded, capacity=%d\n", fifo_capacity());
	return 0;
}

static void __exit kernel_fifo_exit(void)
{
	fifo_clear();
	fifo_destroy();
	pr_info("kernel_fifo: unloaded\n");
}

module_init(kernel_fifo_init);
module_exit(kernel_fifo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("c0un7z3r0");
MODULE_DESCRIPTION("Linux kernel FIFO module based on kfifo and module_param_cb");
MODULE_VERSION("1.0");
