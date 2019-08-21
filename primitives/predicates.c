#include <stdlib.h>
#include <string.h>

#include "predicates.h"
#include "../expr.h"
#include "assert.h"
#include "../exec.h"

int eq(Val val1, Val val2) {
    if (val1.type != val2.type)
        return 0;
    switch (val1.type) {
    case TYPE_INT:
    case TYPE_BOOL:
        return val1.int_data == val2.int_data;
    case TYPE_FLOAT:
        return val1.float_data == val2.float_data;
    case TYPE_STRING:
    case TYPE_SYMBOL:
        return val1.string_data == val2.string_data;
    case TYPE_PRIM:
    case TYPE_HIGH_PRIM:
        return val1.prim_data == val2.prim_data;
    case TYPE_LAMBDA:
        return val1.lambda_data == val2.lambda_data;
    case TYPE_PAIR:
        return val1.pair_data == val2.pair_data;
    case TYPE_NIL:
    case TYPE_VOID:
    case TYPE_BROKEN_HEART:
        return 1;
    }
}

Val eq_prim(Val *args, int num) {
    args_assert(num == 2);
    return (Val){TYPE_BOOL, {.int_data = eq(args[0], args[1])}};
}

int equal(Val val1, Val val2) {
    switch (val1.type) {
    case TYPE_STRING:
        return val2.type == TYPE_STRING && strcmp(val1.string_data, val2.string_data) == 0;
    case TYPE_PAIR:
        return val2.type == TYPE_PAIR
            && equal(val1.pair_data->car, val2.pair_data->car)
            && equal(val1.pair_data->cdr, val2.pair_data->cdr);
    default:
        return eq(val1, val2);
    }
}

Val equal_prim(Val *args, int num) {
    args_assert(num == 2);
    return (Val){TYPE_BOOL, {.int_data = equal(args[0], args[1])}};
}

Val pair_prim(Val *args, int num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_PAIR}};
}

Val null_prim(Val *args, int num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_NIL}};
}

Val number_prim(Val *args, int num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_INT || args[0].type == TYPE_FLOAT}};
}

Val symbol_prim(Val *args, int num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_SYMBOL}};
}

Val string_prim(Val *args, int num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_STRING}};
}

Val procedure_prim(Val *args, int num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_PRIM || args[0].type == TYPE_LAMBDA || args[0].type == TYPE_HIGH_PRIM}};
}

Val not_prim(Val *args, int num) {
    args_assert(num == 1);
    if (is_true(args[0]))
        return (Val){TYPE_BOOL, {.int_data = 0}};
    else
        return (Val){TYPE_BOOL, {.int_data = 1}};
}
