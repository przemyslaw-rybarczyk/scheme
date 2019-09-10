#include <stdlib.h>
#include <string.h>

#include "predicates.h"
#include "../types.h"
#include "../exec_stack.h"
#include "assert.h"

int eq(Val val1, Val val2) {
    if (val1.type != val2.type)
        return 0;
    switch (val1.type) {
    case TYPE_INT:
    case TYPE_BOOL:
        return val1.int_data == val2.int_data;
    case TYPE_FLOAT:
        return val1.float_data == val2.float_data;
    case TYPE_CHAR:
        return val1.char_data == val2.char_data;
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
    case TYPE_UNDEF:
        return 1;
    case TYPE_BROKEN_HEART:
    case TYPE_ENV:
    case TYPE_INST:
    case TYPE_GLOBAL_ENV:
    case TYPE_PRINT_CONTROL:
        return 0;
    }
    return 0;
}

int eqv(Val val1, Val val2) {
    return eq(val1, val2);
}

int equal(Val val1, Val val2) {
    Val *stack_ptr_before = stack_ptr;
    stack_push(val2);
    stack_push(val1);
    while (stack_ptr > stack_ptr_before) {
        val1 = stack_pop();
        val2 = stack_pop();
        if (val1.type != val2.type)
            return 0;
        switch (val1.type) {
        case TYPE_STRING:
            if (val2.type != TYPE_STRING || strcmp(val1.string_data, val2.string_data) != 0) {
                stack_ptr = stack_ptr_before;
                return 0;
            }
            break;
        case TYPE_PAIR:
            stack_push(val2.pair_data->cdr);
            stack_push(val1.pair_data->cdr);
            stack_push(val2.pair_data->car);
            stack_push(val1.pair_data->car);
            break;
        default:
            if (!eqv(val1, val2)) {
                stack_ptr = stack_ptr_before;
                return 0;
            }
            break;
        }
    }
    return 1;
}

Val eq_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    return (Val){TYPE_BOOL, {.int_data = eq(args[0], args[1])}};
}

Val eqv_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    return (Val){TYPE_BOOL, {.int_data = eqv(args[0], args[1])}};
}

Val equal_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    return (Val){TYPE_BOOL, {.int_data = equal(args[0], args[1])}};
}

Val pair_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_PAIR}};
}

Val null_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_NIL}};
}

Val number_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_INT || args[0].type == TYPE_FLOAT}};
}

Val symbol_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_SYMBOL}};
}

Val string_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_STRING}};
}

Val procedure_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_PRIM || args[0].type == TYPE_LAMBDA || args[0].type == TYPE_HIGH_PRIM}};
}

Val boolean_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = args[0].type == TYPE_BOOL}};
}

Val not_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return (Val){TYPE_BOOL, {.int_data = (args[0].type == TYPE_BOOL && args[0].int_data == 0)}};
}
