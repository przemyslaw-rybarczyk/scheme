#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "../expr.h"
#include "assert.h"
#include "../safemem.h"
#include "../memory.h"
#include "../exec.h"
#include "../insts.h"

struct val list_prim(struct val *args, int num) {
    stack_push((struct val){TYPE_NIL});
    for (struct val *arg_ptr = args + num - 1; arg_ptr >= args; arg_ptr--) {
        struct pair *pair = gc_alloc(sizeof(struct pair));
        pair->car = *arg_ptr;
        pair->cdr = stack_pop();
        stack_push((struct val){TYPE_PAIR, {.pair_data = pair}});
    }
    return stack_pop();
}

long long get_length(struct val val0) {
    long long length = 0;
    struct val val = val0;
    for (; val.type == TYPE_PAIR; val = val.data.pair_data->cdr)
        length++;
    if (val.type != TYPE_NIL)
        type_error(val0);
    return length;
}

struct val length_prim(struct val *args, int num) {
    args_assert(num == 1);
    return (struct val){TYPE_INT, {.int_data = get_length(args[0])}};
}

int apply_prim(int num) {
    args_assert(num == 2);
    struct val args = stack_pop();
    struct val proc = stack_pop();
    stack_pop();
    stack_push(proc);
    struct val arg_list = args;
    int arg_num = 0;
    for (; arg_list.type == TYPE_PAIR; arg_list = arg_list.data.pair_data->cdr) {
        stack_push(arg_list.data.pair_data->car);
        arg_num++;
    }
    if (arg_list.type != TYPE_NIL)
        type_error(args);
    insts[tail_call_inst].args.num = arg_num;
    return tail_call_inst;
}
