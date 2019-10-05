#include <string.h>

#include "vector.h"
#include "../types.h"
#include "../memory.h"

static Vector *gc_alloc_vector(size_t len) {
    Vector *vec = gc_alloc(sizeof(Vector) + (len ? len : 1) * sizeof(Val));
    vec->len = len;
    vec->vals[0].type = TYPE_INT;
    return vec;
}

Val vector_prim(Val *args, uint32_t num) {
    Vector *vec = gc_alloc_vector(num);
    memcpy(vec->vals, args, num * sizeof(Val));
    return (Val){TYPE_VECTOR, {.vector_data = vec}};
}
