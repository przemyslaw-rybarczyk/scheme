#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "../expr.h"
#include "assert.h"
#include "../safemem.h"
#include "../memory.h"
#include "../exec.h"
#include "../insts.h"

Val list_prim(Val *args, int num) {
    stack_push((Val){TYPE_NIL});
    for (Val *arg_ptr = args + num - 1; arg_ptr >= args; arg_ptr--) {
        Pair *pair = gc_alloc(sizeof(Pair));
        pair->car = *arg_ptr;
        pair->cdr = stack_pop();
        stack_push((Val){TYPE_PAIR, {.pair_data = pair}});
    }
    return stack_pop();
}

long long get_length(Val val0) {
    long long length = 0;
    Val val = val0;
    for (; val.type == TYPE_PAIR; val = val.pair_data->cdr)
        length++;
    if (val.type != TYPE_NIL)
        type_error(val0);
    return length;
}

Val length_prim(Val *args, int num) {
    args_assert(num == 1);
    return (Val){TYPE_INT, {.int_data = get_length(args[0])}};
}

High_prim_return apply_prim(int num) {
    args_assert(num == 2);
    Val args = stack_pop();
    Val proc = stack_pop();
    stack_pop();
    stack_push(proc);
    Val arg_list = args;
    int arg_num = 0;
    for (; arg_list.type == TYPE_PAIR; arg_list = arg_list.pair_data->cdr) {
        stack_push(arg_list.pair_data->car);
        arg_num++;
    }
    if (arg_list.type != TYPE_NIL)
        type_error(args);
    insts[tail_call_inst].num = arg_num;
    return (High_prim_return){ tail_call_inst, NULL };
}
