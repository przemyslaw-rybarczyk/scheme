#include <stdio.h>
#include <stdlib.h>

#include "../expr.h"
#include "assert.h"
#include "../memory.h"
#include "../eval.h"
#include "../safemem.h"

struct val list_prim(struct val_list *list) {
    if (list == NULL)
        return (struct val){TYPE_NIL};
    struct pair *cdr;
    struct val pair = (struct val){TYPE_PAIR, {.pair_data = alloc_pair()}};
    struct val val = pair;
    gc_push_val(&val);
    gc_push_val(&pair);
    while (list->cdr != NULL) {
        pair.data.pair_data->car = list->car;
        pair.data.pair_data->cdr = (struct val){TYPE_NIL};
        cdr = alloc_pair();
        pair.data.pair_data->cdr = (struct val){TYPE_PAIR, {.pair_data = cdr}};
        pair = pair.data.pair_data->cdr;
        list = list->cdr;
    }
    pair.data.pair_data->car = list->car;
    pair.data.pair_data->cdr = (struct val){TYPE_NIL};
    gc_pop_val();
    gc_pop_val();
    return val;
}

long long get_length(struct val val) {
    switch (val.type) {
    case TYPE_PAIR:
        return get_length(val.data.pair_data->cdr) + 1;
    case TYPE_NIL:
        return 0;
    default:
        fprintf(stderr, "Error: not a list\n");
        exit(2);
    }
}

struct val length_prim(struct val_list *args) {
    assert_1_arg(args);
    return (struct val){TYPE_INT, {.int_data = get_length(args->car)}};
}

struct val_list *list_to_val_list(struct val val) {
    switch (val.type) {
    case TYPE_PAIR: {
        struct val_list *pair = s_malloc(sizeof(struct val_list));
        pair->car = val.data.pair_data->car;
        gc_push_val(&pair->car);
        pair->cdr = list_to_val_list(val.data.pair_data->cdr);
        return pair;
    }
    case TYPE_NIL:
        return NULL;
    default:
        fprintf(stderr, "Error: not a list\n");
        exit(2);
    }
}

struct val apply_prim(struct val_list *args) {
    assert_2_args(args);
    struct val proc = args->car;
    struct val args_list = args->cdr->car;
    free_arg_list(args);
    struct val_list *apply_args = list_to_val_list(args_list);
    return apply(proc, apply_args);
}
