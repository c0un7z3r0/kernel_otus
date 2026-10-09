#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "kernel_alloc_internal.h"

struct memory_allocator g_allocator;

static int __init kernel_alloc_init(void)
{
    int ret;

    ret = allocator_init();
    if (ret != ALLOC_OK) {
        pr_err("kernel_alloc: initialization failed (%d)\n", ret);
        return -ENOMEM;
    }

    pr_info("kernel_alloc: loaded: %zu blocks x %zu bytes = %zu bytes\n",
            g_allocator.total_blocks,
            g_allocator.block_size,
            g_allocator.total_blocks * g_allocator.block_size);
    return 0;
}

static void __exit kernel_alloc_exit(void)
{
    allocator_cleanup();
    pr_info("kernel_alloc: unloaded\n");
}

module_init(kernel_alloc_init);
module_exit(kernel_alloc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("c0un7z3r0");
MODULE_DESCRIPTION("Bitmap-based 10 MiB kernel memory allocator");
MODULE_VERSION("1.0");
