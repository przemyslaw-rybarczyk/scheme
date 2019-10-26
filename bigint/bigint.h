#pragma once

#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

typedef int64_t small_int;
#define SMALL_INT_MIN LLONG_MIN
#define SMALL_INT_MAX LLONG_MAX

#ifdef __SIZEOF_INT128__
    typedef uint64_t bi_base;
    typedef __uint128_t bi_double_base;
    #define BI_BASE_BITS 64
    #define BI_BASE_MAX UINT64_MAX
    #if ULLONG_MAX == UINT64_MAX
        #define clz __builtin_clzll
    #else
        #define clz(x) (__builtin_clzll(x) - ((8 * __SIZEOF_LONG_LONG__) - 64))
    #endif
    #define ctz __builtin_ctzll
#else
    typedef uint32_t bi_base;
    typedef uint64_t bi_double_base;
    #define BI_BASE_BITS 32
    #define BI_BASE_MAX UINT32_MAX
    #if ULONG_MAX == UINT32_MAX
        #define clz __builtin_clzl
    #elif UINT_MAX == UINT32_MAX
        #define clz __builtin_clz
    #else
        #define clz(x) (__builtin_clzl(x) - ((8 * __SIZEOF_LONG__) - 32))
    #endif
    #define ctz __builtin_ctzl
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
int bigint_to_small_int(Bigint *bi, small_int *n);
