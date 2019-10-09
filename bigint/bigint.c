#include "bigint.h"
#include "../memory.h"

Bigint *gc_alloc_bigint(size_t len) {
    size_t digits_size = len * sizeof(bi_base);
    return gc_alloc(sizeof(Bigint) + (digits_size > sizeof(Bigint *) ? digits_size : sizeof(Bigint *)));
}
