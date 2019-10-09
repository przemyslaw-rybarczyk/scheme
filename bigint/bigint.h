#pragma once

#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

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

// Avoid using imaxabs if possible since it doesn't get optimized and compiles
// to a function call.
#if PTRDIFF_MAX <= LLONG_MAX
#define bilabs(x) ((size_t)llabs(x))
#else
#define bilabs(x) ((size_t)imaxabs(x))
#endif

/* -- Bigint
 * Represents an integer consisting of |len| digits. The base is either 2^32
 * or 2^64, depending on whether __uint128_t is available. If len < 0,
 * the number is negative. The digits are stored in little-endian order,
 * and the highest digit is non-zero.
 * The amount of memory allocated for a non-static bigint's digits should
 * be enough to fit a Bigint * , which is used for the purpose of garbage
 * collection.
 */
typedef struct Bigint {
    ptrdiff_t len;
    bi_base digits[];
} Bigint;

Bigint *gc_alloc_bigint(size_t len);
