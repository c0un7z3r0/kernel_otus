#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>

#include "kernel_alloc_internal.h"

static unsigned long long alloc_param;
static unsigned long long free_param;

static int param_alloc_set(const char *val, const struct kernel_param *kp)
{
    unsigned long long bytes;
    void *ptr;

    if (!val)
        return -EINVAL;

    if (kstrtoull(val, 0, &bytes))
        return -EINVAL;

    if (!bytes || bytes > ALLOC_TOTAL_MEMORY)
        return -EINVAL;

    /*
     * The value is intentionally not stored as persistent state: writing the
     * parameter is an operation.  The returned pointer is printed to dmesg
     * and must be supplied to the "free" parameter later.
     */
    ptr = allocator_alloc((size_t)bytes);
    if (!ptr)
        return -ENOMEM;

    alloc_param = (unsigned long long)(unsigned long)ptr;
    pr_info("kernel_alloc: alloc request %llu bytes -> %px\n",
            bytes, ptr);
    return 0;
}

static int param_alloc_get(char *buffer, const struct kernel_param *kp)
{
    return scnprintf(buffer, PAGE_SIZE, "0x%llx\n", alloc_param);
}

static const struct kernel_param_ops alloc_ops = {
    .set = param_alloc_set,
    .get = param_alloc_get,
};

static int param_free_set(const char *val, const struct kernel_param *kp)
{
    unsigned long long address;
    int ret;

    if (!val)
        return -EINVAL;

    if (kstrtoull(val, 0, &address))
        return -EINVAL;

    if (!address)
        return -EINVAL;

    ret = allocator_free((void *)(unsigned long)address);
    if (ret == ALLOC_NOT_FOUND)
        return -ENOENT;
    if (ret != ALLOC_OK)
        return -EINVAL;

    free_param = address;
    return 0;
}

static int param_free_get(char *buffer, const struct kernel_param *kp)
{
    return scnprintf(buffer, PAGE_SIZE, "0x%llx\n", free_param);
}

static const struct kernel_param_ops free_ops = {
    .set = param_free_set,
    .get = param_free_get,
};

static int param_stats_get(char *buffer, const struct kernel_param *kp)
{
    struct stats_info stats;

    stats = allocator_get_stats();

    return scnprintf(buffer, PAGE_SIZE,
                     "Total: %zu KB | Free: %zu KB | Allocated: %zu KB | Fragmentation: %zu%%\n",
                     stats.total_memory / 1024,
                     stats.free_memory / 1024,
                     stats.allocated_memory / 1024,
                     stats.fragmentation_percent);
}

static const struct kernel_param_ops stats_ops = {
    .get = param_stats_get,
};

static int param_kbitmap_info_get(char *buffer, const struct kernel_param *kp)
{
    unsigned long flags;
    size_t i;
    size_t pos = 0;

    if (!g_allocator.bitmap)
        return scnprintf(buffer, PAGE_SIZE, "allocator is not initialized\n");

    spin_lock_irqsave(&g_allocator.lock, flags);

    for (i = 0; i < g_allocator.total_blocks && pos < PAGE_SIZE - 2; ++i)
        buffer[pos++] = kbitmap_test(g_allocator.bitmap, i) ? 'X' : '.';

    spin_unlock_irqrestore(&g_allocator.lock, flags);

    buffer[pos++] = '\n';
    buffer[pos] = '\0';
    return pos;
}

static const struct kernel_param_ops kbitmap_info_ops = {
    .get = param_kbitmap_info_get,
};

module_param_cb(alloc, &alloc_ops, &alloc_param, 0600);
MODULE_PARM_DESC(alloc, "Allocate bytes using the bitmap allocator");

module_param_cb(free, &free_ops, &free_param, 0600);
MODULE_PARM_DESC(free, "Free an allocation by its returned kernel virtual address");

module_param_cb(stats, &stats_ops, NULL, 0444);
MODULE_PARM_DESC(stats, "Allocator statistics");

module_param_cb(kbitmap_info, &kbitmap_info_ops, NULL, 0444);
MODULE_PARM_DESC(kbitmap_info, "Bitmap: X=allocated block, .=free block");

int params_init(void)
{
    return 0;
}

void params_exit(void)
{
}
