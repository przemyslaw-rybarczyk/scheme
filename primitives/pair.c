#include <stdio.h>
#include <stdlib.h>

#include "../expr.h"
#include "assert.h"
#include "../memory.h"

struct val cons_prim(struct val_list *args) {
    assert_2_args(args);
    struct pair *pair = alloc_pair();
    pair->car = args->car;
    pair->cdr = args->cdr->car;
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

struct val car_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(args->car);
}

struct val caar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_car(args->car));
}

struct val caaar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_car(get_car(args->car)));
}

struct val caaaar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_car(get_car(get_car(args->car))));
}

struct val cdaaar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_car(get_car(get_car(args->car))));
}

struct val cdaar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_car(get_car(args->car)));
}

struct val cadaar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_cdr(get_car(get_car(args->car))));
}

struct val cddaar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_cdr(get_car(get_car(args->car))));
}

struct val cdar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_car(args->car));
}

struct val cadar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_cdr(get_car(args->car)));
}

struct val caadar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_car(get_cdr(get_car(args->car))));
}

struct val cdadar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_car(get_cdr(get_car(args->car))));
}

struct val cddar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_cdr(get_car(args->car)));
}

struct val caddar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_cdr(get_cdr(get_car(args->car))));
}

struct val cdddar_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_cdr(get_cdr(get_car(args->car))));
}

struct val cdr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(args->car);
}

struct val cadr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_cdr(args->car));
}

struct val caadr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_car(get_cdr(args->car)));
}

struct val caaadr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_car(get_car(get_cdr(args->car))));
}

struct val cdaadr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_car(get_car(get_cdr(args->car))));
}

struct val cdadr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_car(get_cdr(args->car)));
}

struct val cadadr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_cdr(get_car(get_cdr(args->car))));
}

struct val cddadr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_cdr(get_car(get_cdr(args->car))));
}

struct val cddr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_cdr(args->car));
}

struct val caddr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_cdr(get_cdr(args->car)));
}

struct val caaddr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_car(get_cdr(get_cdr(args->car))));
}

struct val cdaddr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_car(get_cdr(get_cdr(args->car))));
}

struct val cdddr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_cdr(get_cdr(args->car)));
}

struct val cadddr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_car(get_cdr(get_cdr(get_cdr(args->car))));
}

struct val cddddr_prim(struct val_list *args) {
    assert_1_arg(args);
    return get_cdr(get_cdr(get_cdr(get_cdr(args->car))));
}

struct val set_car_prim(struct val_list *args) {
    assert_2_args(args);
    if (args->car.type != TYPE_PAIR)
        type_error(args->car);
    args->car.data.pair_data->car = args->cdr->car;
    return (struct val){TYPE_VOID};
}

struct val set_cdr_prim(struct val_list *args) {
    assert_2_args(args);
    if (args->car.type != TYPE_PAIR)
        type_error(args->car);
    args->car.data.pair_data->cdr = args->cdr->car;
    return (struct val){TYPE_VOID};
}
