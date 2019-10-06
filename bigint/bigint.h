#include <inttypes.h>

#ifdef __SIZEOF_INT128__
typedef uint64_t base;
typedef __uint128_t double_base;
#else
typedef uint32_t base;
typedef uint64_t double_base;
#endif

typedef struct Bigint {
    size_t len;
    base digits[];
} Bigint;
