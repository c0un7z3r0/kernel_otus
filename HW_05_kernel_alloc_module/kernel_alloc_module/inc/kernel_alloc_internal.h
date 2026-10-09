#ifndef KERNEL_ALLOC_INTERNAL_H
#define KERNEL_ALLOC_INTERNAL_H

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/list.h>

#define ALLOC_OK        0
#define ALLOC_NOMEM     -1
#define ALLOC_INVALID   -2
#define ALLOC_NOT_FOUND -3

#define ALLOC_TOTAL_MEMORY (10UL * 1024UL * 1024UL)
#define ALLOC_BLOCK_SIZE   (4UL * 1024UL)
#define ALLOC_TOTAL_BLOCKS (ALLOC_TOTAL_MEMORY / ALLOC_BLOCK_SIZE)
#define ALLOC_kbitmap_BYTES ((ALLOC_TOTAL_BLOCKS + 7UL) / 8UL)

struct memory_allocator {
    unsigned char *bitmap;
    void *memory_pool;
    size_t total_blocks;
    size_t block_size;
    spinlock_t lock;
};

struct allocation_info {
    size_t start_block;
    size_t num_blocks;
    struct list_head list;
};

struct stats_info {
    size_t total_blocks;
    size_t free_blocks;
    size_t allocated_blocks;
    size_t total_memory;
    size_t free_memory;
    size_t allocated_memory;
    size_t fragmentation_percent;
};

extern struct memory_allocator g_allocator;

int allocator_init(void);
void *allocator_alloc(size_t bytes);
int allocator_free(void *ptr);
struct stats_info allocator_get_stats(void);
void allocator_cleanup(void);

int kbitmap_test(const unsigned char *bitmap, size_t bit);
void kbitmap_set(unsigned char *bitmap, size_t bit);
void kbitmap_clear(unsigned char *bitmap, size_t bit);
size_t kbitmap_find_first_fit(const unsigned char *bitmap, size_t nbits,
                             size_t needed);

int params_init(void);
void params_exit(void);

#endif
