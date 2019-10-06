#pragma once

#include <inttypes.h>
#include <stddef.h>

extern void *gc_alloc(size_t size);

#ifdef __SIZEOF_INT128__
typedef uint64_t bi_base;
typedef __uint128_t bi_double_base;
#define BI_BASE_BITS 64
#define BI_BASE_MAX UINT64_MAX
#else
typedef uint32_t bi_base;
typedef uint64_t bi_double_base;
#define BI_BASE_BITS 32
#define BI_BASE_MAX UINT32_MAX
#endif

/* -- Bigint
 * Represents an integer consisting of |len| digits. The base is either 2^32
 * or 2^64, depending on whether __uint128_t is available. If len < 0,
 * the number is negative. The digits are stored in little-endian order,
 * and the highest digit is non-zero.
 */
typedef struct Bigint {
    ptrdiff_t len;
    bi_base digits[];
} Bigint;
