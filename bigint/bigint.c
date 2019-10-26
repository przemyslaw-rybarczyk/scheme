#include "bigint.h"
#include "../types.h"
#include "ops.h"
#include "../memory.h"

Bigint *gc_alloc_bigint(size_t len) {
    size_t digits_size = len * sizeof(bi_base);
    return gc_alloc(sizeof(Bigint) + (digits_size > sizeof(Bigint *) ? digits_size : sizeof(Bigint *)));
}

int bigint_to_small_int(Bigint *bi, small_int *n) {
    if (bilabs(bi->len) > 64 / BI_BASE_BITS)
        return 0;
#if BI_BASE_BITS == 64
    bi_base unsigned_n = bi->len ? bi->digits[0] : 0;
#else
    bi_double_base unsigned_n == 0;
    for (int i = 0; i < bilabs(bi->len))
        unsigned_n = (unsigned_n << BI_BASE_BITS) | bi->digits[i];
#endif
    if (unsigned_n > (bi->len < 0 ? -SMALL_INT_MIN : SMALL_INT_MAX))
        return 0;
    *n = (small_int)unsigned_n;
    return 1;
}
