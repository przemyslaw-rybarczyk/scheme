#include <inttypes.h>

typedef uint64_t base;

typedef struct Bigint {
    size_t len;
    base digits[];
} Bigint;
