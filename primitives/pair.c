#include <stdio.h>
#include <stdlib.h>

#include "pair.h"
#include "../expr.h"
#include "assert.h"
#include "../memory.h"

struct val cons_prim(struct val *args, int num) {
    args_assert(num == 2);
    struct pair *pair = alloc_pair();
    pair->car = args[0];
    pair->cdr = args[1];
    return (struct val){TYPE_PAIR, {.pair_data = pair}};
}

struct val get_car(struct val pair) {
    if (pair.type != TYPE_PAIR) {
        fprintf(stderr, "Error: not a pair\n");
        exit(2);
    }
    return pair.data.pair_data->car;
}

struct val get_cdr(struct val pair) {
    if (pair.type != TYPE_PAIR) {
        fprintf(stderr, "Error: not a pair\n");
        exit(2);
    }
    return pair.data.pair_data->cdr;
}

struct val car_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(args[0]);
}

struct val caar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_car(args[0]));
}

struct val caaar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_car(get_car(args[0])));
}

struct val caaaar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_car(get_car(get_car(args[0]))));
}

struct val cdaaar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_car(get_car(args[0]))));
}

struct val cdaar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_car(args[0])));
}

struct val cadaar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_car(get_car(args[0]))));
}

struct val cddaar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_car(get_car(args[0]))));
}

struct val cdar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_car(args[0]));
}

struct val cadar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_car(args[0])));
}

struct val caadar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_car(get_cdr(get_car(args[0]))));
}

struct val cdadar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_cdr(get_car(args[0]))));
}

struct val cddar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_car(args[0])));
}

struct val caddar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_cdr(get_car(args[0]))));
}

struct val cdddar_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_cdr(get_car(args[0]))));
}

struct val cdr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(args[0]);
}

struct val cadr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_cdr(args[0]));
}

struct val caadr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_car(get_cdr(args[0])));
}

struct val caaadr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_car(get_car(get_cdr(args[0]))));
}

struct val cdaadr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_car(get_cdr(args[0]))));
}

struct val cdadr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_cdr(args[0])));
}

struct val cadadr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_car(get_cdr(args[0]))));
}

struct val cddadr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_car(get_cdr(args[0]))));
}

struct val cddr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(args[0]));
}

struct val caddr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_cdr(args[0])));
}

struct val caaddr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_car(get_cdr(get_cdr(args[0]))));
}

struct val cdaddr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_cdr(get_cdr(args[0]))));
}

struct val cdddr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_cdr(args[0])));
}

struct val cadddr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_cdr(get_cdr(args[0]))));
}

struct val cddddr_prim(struct val *args, int num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_cdr(get_cdr(args[0]))));
}

struct val set_car_prim(struct val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_PAIR)
        type_error(args[0]);
    args[0].data.pair_data->car = args[1];
    return (struct val){TYPE_VOID};
}

struct val set_cdr_prim(struct val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_PAIR)
        type_error(args[0]);
    args[0].data.pair_data->cdr = args[1];
    return (struct val){TYPE_VOID};
}
