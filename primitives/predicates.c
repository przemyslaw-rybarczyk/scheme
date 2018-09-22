#include <stdlib.h>
#include <string.h>

#include "../expr.h"
#include "assert.h"
#include "../eval.h"

int eq(struct val val1, struct val val2) {
    if (val1.type != val2.type)
        return 0;
    switch (val1.type) {
    case TYPE_INT:
    case TYPE_BOOL:
        return val1.data.int_data == val2.data.int_data;
    case TYPE_FLOAT:
        return val1.data.float_data == val2.data.float_data;
    case TYPE_STRING:
    case TYPE_SYMBOL:
        return val1.data.string_data == val2.data.string_data;
    case TYPE_PRIM:
    case TYPE_HIGH_PRIM:
        return val1.data.prim_data == val2.data.prim_data;
    case TYPE_LAMBDA:
        return val1.data.lambda_data == val2.data.lambda_data;
    case TYPE_PAIR:
        return val1.data.pair_data == val2.data.pair_data;
    case TYPE_NIL:
    case TYPE_VOID:
    case TYPE_BROKEN_HEART:
        return 1;
    }
}

struct val eq_prim(struct val_list *args) {
    assert_2_args(args);
    return (struct val){TYPE_BOOL, {.int_data = eq(args->car, args->cdr->car)}};
}

int equal(struct val val1, struct val val2) {
    switch (val1.type) {
    case TYPE_STRING:
        return val2.type == TYPE_STRING && strcmp(val1.data.string_data, val2.data.string_data) == 0;
    case TYPE_PAIR:
        return val2.type == TYPE_PAIR
            && equal(val1.data.pair_data->car, val2.data.pair_data->car)
            && equal(val1.data.pair_data->cdr, val2.data.pair_data->cdr);
    default:
        return eq(val1, val2);
    }
}

struct val equal_prim(struct val_list *args) {
    assert_2_args(args);
    return (struct val){TYPE_BOOL, {.int_data = equal(args->car, args->cdr->car)}};
}

struct val pair_prim(struct val_list *args) {
    assert_1_arg(args);
    return (struct val){TYPE_BOOL, {.int_data = args->car.type == TYPE_PAIR}};
}

struct val null_prim(struct val_list *args) {
    assert_1_arg(args);
    return (struct val){TYPE_BOOL, {.int_data = args->car.type == TYPE_NIL}};
}

struct val number_prim(struct val_list *args) {
    assert_1_arg(args);
    return (struct val){TYPE_BOOL, {.int_data = args->car.type == TYPE_INT || args->car.type == TYPE_FLOAT}};
}

struct val symbol_prim(struct val_list *args) {
    assert_1_arg(args);
    return (struct val){TYPE_BOOL, {.int_data = args->car.type == TYPE_SYMBOL}};
}

struct val string_prim(struct val_list *args) {
    assert_1_arg(args);
    return (struct val){TYPE_BOOL, {.int_data = args->car.type == TYPE_STRING}};
}

struct val not_prim(struct val_list *args) {
    assert_1_arg(args);
    if (is_true(args->car))
        return (struct val){TYPE_BOOL, {.int_data = 0}};
    else
        return (struct val){TYPE_BOOL, {.int_data = 1}};
}
