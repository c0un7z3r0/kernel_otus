#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "kernel_alloc_internal.h"

/*
 * The bitmap alone cannot safely identify the length of an allocation:
 * adjacent allocations may occupy consecutive bits.  Therefore we keep a
 * small allocation descriptor list.  The bitmap remains the source of truth
 * for block occupancy, while the list maps an exact returned pointer to its
 * allocation length.
 */
static LIST_HEAD(allocation_list);

static void kbitmap_mark_range(size_t start, size_t count, bool used)
{
    size_t i;

    for (i = 0; i < count; ++i) {
        if (used)
            kbitmap_set(g_allocator.bitmap, start + i);
        else
            kbitmap_clear(g_allocator.bitmap, start + i);
    }
}

static struct allocation_info *find_allocation_locked(void *ptr)
{
    struct allocation_info *info;
    unsigned long ptr_addr;
    unsigned long pool_addr;
    unsigned long offset;

    if (!ptr || !g_allocator.memory_pool)
        return NULL;

    ptr_addr = (unsigned long)(unsigned long)ptr;
    pool_addr = (unsigned long)(unsigned long)g_allocator.memory_pool;

    if (ptr_addr < pool_addr)
        return NULL;

    offset = ptr_addr - pool_addr;
    if (offset >= g_allocator.total_blocks * g_allocator.block_size)
        return NULL;
    if (offset % g_allocator.block_size)
        return NULL;

    list_for_each_entry(info, &allocation_list, list) {
        if (info->start_block * g_allocator.block_size == offset)
            return info;
    }

    return NULL;
}

int allocator_init(void)
{
    memset(&g_allocator, 0, sizeof(g_allocator));
    g_allocator.total_blocks = ALLOC_TOTAL_BLOCKS;
    g_allocator.block_size = ALLOC_BLOCK_SIZE;
    spin_lock_init(&g_allocator.lock);

    g_allocator.bitmap = kzalloc(ALLOC_kbitmap_BYTES, GFP_KERNEL);
    if (!g_allocator.bitmap)
        return ALLOC_NOMEM;

    /*
     * vmalloc is appropriate here because the allocator manages a virtual
     * kernel-memory pool; physical contiguity is not required.
     */
    g_allocator.memory_pool = vmalloc(ALLOC_TOTAL_MEMORY);
    if (!g_allocator.memory_pool) {
        kfree(g_allocator.bitmap);
        g_allocator.bitmap = NULL;
        return ALLOC_NOMEM;
    }

    INIT_LIST_HEAD(&allocation_list);
    return ALLOC_OK;
}

void *allocator_alloc(size_t bytes)
{
    struct allocation_info *info;
    unsigned long flags;
    size_t blocks;
    size_t start;
    void *ptr;

    if (!bytes)
        return NULL;

    blocks = DIV_ROUND_UP(bytes, g_allocator.block_size);
    if (!blocks || blocks > g_allocator.total_blocks)
        return NULL;

    info = kmalloc(sizeof(*info), GFP_KERNEL);
    if (!info)
        return NULL;

    spin_lock_irqsave(&g_allocator.lock, flags);

    start = kbitmap_find_first_fit(g_allocator.bitmap,
                                  g_allocator.total_blocks, blocks);
    if (start == g_allocator.total_blocks) {
        spin_unlock_irqrestore(&g_allocator.lock, flags);
        kfree(info);
        return NULL;
    }

    kbitmap_mark_range(start, blocks, true);

    info->start_block = start;
    info->num_blocks = blocks;
    list_add_tail(&info->list, &allocation_list);

    ptr = (char *)g_allocator.memory_pool + start * g_allocator.block_size;

    spin_unlock_irqrestore(&g_allocator.lock, flags);

    pr_info("kernel_alloc: allocated %zu bytes (%zu blocks) at %px\n",
            blocks * g_allocator.block_size, blocks, ptr);
    return ptr;
}

int allocator_free(void *ptr)
{
    struct allocation_info *info;
    unsigned long flags;

    if (!ptr)
        return ALLOC_INVALID;

    spin_lock_irqsave(&g_allocator.lock, flags);

    info = find_allocation_locked(ptr);
    if (!info) {
        spin_unlock_irqrestore(&g_allocator.lock, flags);
        return ALLOC_NOT_FOUND;
    }

    kbitmap_mark_range(info->start_block, info->num_blocks, false);
    list_del(&info->list);

    spin_unlock_irqrestore(&g_allocator.lock, flags);

    pr_info("kernel_alloc: freed memory at %px (%zu blocks)\n",
            ptr, info->num_blocks);
    kfree(info);

    return ALLOC_OK;
}

struct stats_info allocator_get_stats(void)
{
    struct stats_info stats = {0};
    struct allocation_info *info;
    unsigned long flags;
    size_t i;
    size_t largest_free_run = 0;
    size_t current_free_run = 0;

    stats.total_blocks = g_allocator.total_blocks;
    stats.total_memory = g_allocator.total_blocks * g_allocator.block_size;

    spin_lock_irqsave(&g_allocator.lock, flags);

    list_for_each_entry(info, &allocation_list, list)
        stats.allocated_blocks += info->num_blocks;

    stats.free_blocks = stats.total_blocks - stats.allocated_blocks;
    stats.allocated_memory = stats.allocated_blocks * g_allocator.block_size;
    stats.free_memory = stats.free_blocks * g_allocator.block_size;

    /*
     * External fragmentation definition used by this module:
     *   100 * (free_blocks - largest_free_run) / free_blocks
     * A completely free pool therefore has 0% fragmentation.
     */
    for (i = 0; i < stats.total_blocks; ++i) {
        if (!kbitmap_test(g_allocator.bitmap, i)) {
            ++current_free_run;
            if (current_free_run > largest_free_run)
                largest_free_run = current_free_run;
        } else {
            current_free_run = 0;
        }
    }

    if (stats.free_blocks && largest_free_run < stats.free_blocks)
        stats.fragmentation_percent =
            ((stats.free_blocks - largest_free_run) * 100) /
            stats.free_blocks;

    spin_unlock_irqrestore(&g_allocator.lock, flags);

    return stats;
}

void allocator_cleanup(void)
{
    struct allocation_info *info;
    struct allocation_info *tmp;
    unsigned long flags;

    spin_lock_irqsave(&g_allocator.lock, flags);

    list_for_each_entry_safe(info, tmp, &allocation_list, list) {
        list_del(&info->list);
        kfree(info);
    }

    if (g_allocator.bitmap) {
        kfree(g_allocator.bitmap);
        g_allocator.bitmap = NULL;
    }

    spin_unlock_irqrestore(&g_allocator.lock, flags);

    if (g_allocator.memory_pool) {
        vfree(g_allocator.memory_pool);
        g_allocator.memory_pool = NULL;
    }

    g_allocator.total_blocks = 0;
    g_allocator.block_size = 0;
}
