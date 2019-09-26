#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "../types.h"
#include "../exec_stack.h"
#include "../insts.h"
#include "../memory.h"
#include "assert.h"
#include "predicates.h"

Val list_q_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    Val list = args[0];
    uint32_t length = 0;
    // Visited pairs are marked by setting their cdr's type to TYPE_BROKEN_HEART;
    // This is done to detect cyclical lists.
    while (list.type == TYPE_PAIR && list.pair_data->cdr.type != TYPE_BROKEN_HEART) {
        Val list_ = list.pair_data->cdr;
        list.pair_data->cdr.type = TYPE_BROKEN_HEART;
        stack_push(list);
        list = list_;
        length++;
    }
    int result = (list.type == TYPE_NIL);
    for (; length > 0; length--) {
        Val list_ = stack_pop();
        list_.pair_data->cdr.type = list.type;
        list = list_;
    }
    return (Val){TYPE_BOOL, {.int_data = result}};
}

Val list_prim(Val *args, uint32_t num) {
    stack_push((Val){TYPE_NIL});
    for (Val *arg_ptr = args + num - 1; arg_ptr >= args; arg_ptr--) {
        Pair *pair = gc_alloc(sizeof(Pair));
        pair->car = *arg_ptr;
        pair->cdr = stack_pop();
        stack_push((Val){TYPE_PAIR, {.pair_data = pair}});
    }
    return stack_pop();
}

Val length_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    long long length = 0;
    Val val = args[0];
    for (; val.type == TYPE_PAIR; val = val.pair_data->cdr)
        length++;
    if (val.type != TYPE_NIL)
        type_error(val);
    return (Val){TYPE_INT, {.int_data = length}};
}

Val append_prim(Val *args, uint32_t num) {
    if (num == 0)
        return (Val){TYPE_NIL};
    Val val = args[num - 1];
    for (int32_t i = (int32_t)num - 2; i >= 0; i--) {
        Val arg = args[i];
        uint32_t length = 0;
        while (arg.type == TYPE_PAIR) {
            stack_push(arg.pair_data->car);
            arg = arg.pair_data->cdr;
            length++;
        }
        if (arg.type != TYPE_NIL)
            type_error(arg);
        stack_push(val);
        for (uint32_t j = 0; j < length; j++) {
            Pair *pair = gc_alloc(sizeof(Pair));
            pair->cdr = stack_pop();
            pair->car = stack_pop();
            stack_push((Val){TYPE_PAIR, {.pair_data = pair}});
        }
        val = stack_pop();
    }
    return val;
}

Val reverse_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    Val tail = args[0];
    stack_push((Val){TYPE_NIL});
    stack_push(tail);
    while (tail.type == TYPE_PAIR) {
        Pair *pair = gc_alloc(sizeof(Pair));
        tail = stack_pop();
        pair->car = tail.pair_data->car;
        pair->cdr = stack_pop();
        tail = tail.pair_data->cdr;
        stack_push((Val){TYPE_PAIR, {.pair_data = pair}});
        stack_push(tail);
    }
    if (tail.type != TYPE_NIL)
        type_error(tail);
    stack_pop();
    return stack_pop();
}

Val list_tail_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    long long k = args[1].int_data;
    if (k < 0) {
        eprintf("Error: list index less than zero\n");
        exit(1);
    }
    Val list = args[0];
    for (; k > 0; k--) {
        if (list.type != TYPE_PAIR)
            type_error(list);
        list = list.pair_data->cdr;
    }
    return list;
}

Val list_ref_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    long long k = args[1].int_data;
    if (k < 0) {
        eprintf("Error: list index less than zero\n");
        exit(1);
    }
    Val list = args[0];
    for (; k > 0; k--) {
        if (list.type != TYPE_PAIR)
            type_error(list);
        list = list.pair_data->cdr;
    }
    if (list.type != TYPE_PAIR)
        type_error(list);
    return list.pair_data->car;
}

#define def_member_prim(name, f) \
Val name(Val *args, uint32_t num) { \
    args_assert(num == 2); \
    Val obj = args[0]; \
    Val list = args[1]; \
    while (list.type == TYPE_PAIR) { \
        if (f(list.pair_data->car, obj)) \
            return list; \
        list = list.pair_data->cdr; \
    } \
    if (list.type != TYPE_NIL) \
        type_error(list); \
    return (Val){TYPE_BOOL, {.int_data = 0}}; \
}

def_member_prim(memq_prim, eq)
def_member_prim(memv_prim, eqv)
def_member_prim(member_prim, equal)

#define def_assoc_prim(name, f) \
Val name(Val *args, uint32_t num) { \
    args_assert(num == 2); \
    Val obj = args[0]; \
    Val list = args[1]; \
    while (list.type == TYPE_PAIR) { \
        if (list.pair_data->car.type != TYPE_PAIR) \
            type_error(list.pair_data->car); \
        if (f(list.pair_data->car.pair_data->car, obj)) \
            return list.pair_data->car; \
        list = list.pair_data->cdr; \
    } \
    if (list.type != TYPE_NIL) \
        type_error(list); \
    return (Val){TYPE_BOOL, {.int_data = 0}}; \
}

def_assoc_prim(assq_prim, eq)
def_assoc_prim(assv_prim, eqv)
def_assoc_prim(assoc_prim, equal)

High_prim_return map_prim_continuation(Val *args, uint32_t num);

High_prim_return map_prim(Val *args, uint32_t num) {
    args_assert(num > 1);
    stack_push((Val){TYPE_INT, {.int_data = num - 1}});
    stack_push((Val){TYPE_INT, {.int_data = 1}});
    stack_push(args[0]);
    for (uint32_t i = 1; i < num; i++) {
        if (args[i].type != TYPE_PAIR) {
            stack_ptr = args - 1;
            stack_push((Val){TYPE_NIL});
            return (High_prim_return){return_inst, NULL};
        }
        stack_push(args[i].pair_data->car);
        args[i] = args[i].pair_data->cdr;
    }
    insts[map_continue_inst].num = num - 1;
    return (High_prim_return){map_continue_inst, NULL};
}

High_prim_return map_prim_continuation(Val *args, uint32_t num) {
    stack_pop();
    Val last_val = stack_pop();
    uint32_t k = (uint32_t)stack_pop().int_data;
    stack_push(last_val);
    stack_push((Val){TYPE_INT, {.int_data = k + 1}});
    args -= k + 3;
    uint32_t n = (uint32_t)args[0].int_data;
    args -= n + 1;
    stack_push(args[0]);
    for (uint32_t i = 1; i <= n; i++) {
        if (args[i].type != TYPE_PAIR) {
            stack_ptr = args + n + k + 2;
            stack_push((Val){TYPE_NIL});
            for (uint32_t j = k; j > 0; j--) {
                Pair *pair = gc_alloc(sizeof(Pair));
                pair->cdr = stack_pop();
                pair->car = stack_pop();
                stack_push((Val){TYPE_PAIR, {.pair_data = pair}});
            }
            Val val = stack_pop();
            stack_ptr = args - 1;
            stack_push(val);
            return (High_prim_return){return_inst, NULL};
        }
        stack_push(args[i].pair_data->car);
        args[i] = args[i].pair_data->cdr;
    }
    insts[map_continue_inst].num = n;
    return (High_prim_return){map_continue_inst, NULL};
}

High_prim_return for_each_prim(Val *args, uint32_t num) {
    args_assert(num > 1);
    stack_push((Val){TYPE_INT, {.int_data = num}});
    stack_push(args[0]);
    for (uint32_t i = 1; i < num; i++) {
        if (args[i].type != TYPE_PAIR) {
            stack_ptr = args - 1;
            stack_push((Val){TYPE_VOID});
            return (High_prim_return){return_inst, NULL};
        }
        stack_push(args[i].pair_data->car);
        args[i] = args[i].pair_data->cdr;
    }
    insts[for_each_continue_inst].num = num - 1;
    return (High_prim_return){for_each_continue_inst, NULL};
}

High_prim_return for_each_prim_continuation(Val *args, uint32_t num) {
    stack_pop();
    stack_pop();
    uint32_t n = (uint32_t)stack_pop().int_data;
    return for_each_prim(args - n - 3, n);
}

High_prim_return apply_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    Val arg_list0 = stack_pop();
    Val proc = stack_pop();
    stack_pop();
    stack_push(proc);
    Val arg_list = arg_list0;
    uint32_t arg_num = 0;
    for (; arg_list.type == TYPE_PAIR; arg_list = arg_list.pair_data->cdr) {
        stack_push(arg_list.pair_data->car);
        arg_num++;
    }
    if (arg_list.type != TYPE_NIL)
        type_error(arg_list0);
    insts[tail_call_inst].num = arg_num;
    return (High_prim_return){tail_call_inst, NULL};
}
