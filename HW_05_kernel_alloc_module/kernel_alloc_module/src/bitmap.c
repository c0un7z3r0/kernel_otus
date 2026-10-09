#include <linux/bitops.h>
#include <linux/kernel.h>
#include "kernel_alloc_internal.h"

int kbitmap_test(const unsigned char *bitmap, size_t bit)
{
    return !!(bitmap[bit / 8] & (1U << (bit % 8)));
}

void kbitmap_set(unsigned char *bitmap, size_t bit)
{
    bitmap[bit / 8] |= (unsigned char)(1U << (bit % 8));
}

void kbitmap_clear(unsigned char *bitmap, size_t bit)
{
    bitmap[bit / 8] &= (unsigned char)~(1U << (bit % 8));
}

size_t kbitmap_find_first_fit(const unsigned char *bitmap, size_t nbits,
                             size_t needed)
{
    size_t i;
    size_t run = 0;

    if (!needed || needed > nbits)
        return nbits;

    for (i = 0; i < nbits; ++i) {
        if (!kbitmap_test(bitmap, i)) {
            ++run;
            if (run == needed)
                return i + 1 - needed;
        } else {
            run = 0;
        }
    }

    return nbits;
}
