#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "../types.h"
#include "../exec_stack.h"
#include "../memory.h"
#include "assert.h"

static Vector *gc_alloc_vector(size_t len) {
    Vector *vec = gc_alloc(sizeof(Vector) + (len ? len : 1) * sizeof(Val));
    vec->len = len;
    vec->vals[0].type = TYPE_INT;
    return vec;
}

Val make_vector_prim(Val *args, uint32_t num) {
    args_assert(num == 1 || num == 2);
    Val e = (Val){TYPE_VOID};
    if (num == 2)
        e = args[1];
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[0].int_data < 0) {
        eprintf("Error: negative length of vector provided\n");
        exit(1);
    }
    Vector *vec = gc_alloc_vector((size_t)args[0].int_data);
    for (size_t i = 0; i < args[0].int_data; i++)
        vec->vals[i] = e;
    return (Val){TYPE_VECTOR, {.vector_data = vec}};
}

Val vector_prim(Val *args, uint32_t num) {
    Vector *vec = gc_alloc_vector(num);
    memcpy(vec->vals, args, num * sizeof(Val));
    return (Val){TYPE_VECTOR, {.vector_data = vec}};
}

Val vector_length_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_VECTOR && args[0].type != TYPE_CONST_VECTOR)
        type_error(args[0]);
    return (Val){TYPE_INT, {.int_data = (long long)args[0].vector_data->len}};
}

Val vector_ref_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_VECTOR && args[0].type != TYPE_CONST_VECTOR)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    if (args[1].int_data < 0 || args[1].int_data >= args[0].vector_data->len) {
        eprintf("Error: vector index out of range\n");
        exit(1);
    }
    return args[0].vector_data->vals[args[1].int_data];
}

Val vector_set_prim(Val *args, uint32_t num) {
    args_assert(num == 3);
    if (args[0].type == TYPE_CONST_VECTOR) {
        eprintf("Error: vector is immutable\n");
        exit(1);
    }
    if (args[0].type != TYPE_VECTOR)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    if (args[1].int_data < 0 || args[1].int_data >= args[0].vector_data->len) {
        eprintf("Error: vector index out of range\n");
        exit(1);
    }
    args[0].vector_data->vals[args[1].int_data] = args[2];
    return (Val){TYPE_VOID};
}

Val vector_to_list_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_VECTOR && args[0].type != TYPE_CONST_VECTOR)
        type_error(args[0]);
    Val *arg = stack_ptr;
    stack_push(args[0]);
    stack_push((Val){TYPE_NIL});
    for (ssize_t i = (ssize_t)arg->vector_data->len - 1; i >= 0; i--) {
        Pair *pair = gc_alloc(sizeof(Pair));
        pair->car = arg->vector_data->vals[i];
        pair->cdr = stack_pop();
        stack_push((Val){TYPE_PAIR, {.pair_data = pair}});
    }
    return stack_pop();
}

Val list_to_vector_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    size_t len = 0;
    Val list = args[0];
    for (; list.type == TYPE_PAIR || list.type == TYPE_CONST_PAIR; list = list.pair_data->cdr)
        len++;
    if (list.type != TYPE_NIL)
        type_error(list);
    list = args[0];
    Vector *vec = gc_alloc_vector(len);
    for (size_t i = 0; i < len; i++) {
        vec->vals[i] = list.pair_data->car;
        list = list.pair_data->cdr;
    }
    return (Val){TYPE_VECTOR, {.vector_data = vec}};
}

Val vector_fill_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type == TYPE_CONST_VECTOR) {
        eprintf("Error: vector is immutable\n");
        exit(1);
    }
    if (args[0].type != TYPE_VECTOR)
        type_error(args[0]);
    for (size_t i = 0; i < args[0].vector_data->len; i++)
        args[0].vector_data->vals[i] = args[1];
    return (Val){TYPE_VOID};
}
