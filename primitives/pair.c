#include <stdio.h>
#include <stdlib.h>

#include "pair.h"
#include "../types.h"
#include "../memory.h"
#include "assert.h"

Val cons_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    Pair *pair = gc_alloc(sizeof(Pair));
    pair->car = args[0];
    pair->cdr = args[1];
    return (Val){TYPE_PAIR, {.pair_data = pair}};
}

Val get_car(Val pair) {
    if (pair.type != TYPE_PAIR && pair.type != TYPE_CONST_PAIR) {
        eprintf("Error: not a pair\n");
        exit(1);
    }
    return pair.pair_data->car;
}

Val get_cdr(Val pair) {
    if (pair.type != TYPE_PAIR && pair.type != TYPE_CONST_PAIR) {
        eprintf("Error: not a pair\n");
        exit(1);
    }
    return pair.pair_data->cdr;
}

Val car_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(args[0]);
}

Val caar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_car(args[0]));
}

Val caaar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_car(get_car(args[0])));
}

Val caaaar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_car(get_car(get_car(args[0]))));
}

Val cdaaar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_car(get_car(args[0]))));
}

Val cdaar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_car(args[0])));
}

Val cadaar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_car(get_car(args[0]))));
}

Val cddaar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_car(get_car(args[0]))));
}

Val cdar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_car(args[0]));
}

Val cadar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_car(args[0])));
}

Val caadar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_car(get_cdr(get_car(args[0]))));
}

Val cdadar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_cdr(get_car(args[0]))));
}

Val cddar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_car(args[0])));
}

Val caddar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_cdr(get_car(args[0]))));
}

Val cdddar_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_cdr(get_car(args[0]))));
}

Val cdr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(args[0]);
}

Val cadr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_cdr(args[0]));
}

Val caadr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_car(get_cdr(args[0])));
}

Val caaadr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_car(get_car(get_cdr(args[0]))));
}

Val cdaadr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_car(get_cdr(args[0]))));
}

Val cdadr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_cdr(args[0])));
}

Val cadadr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_car(get_cdr(args[0]))));
}

Val cddadr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_car(get_cdr(args[0]))));
}

Val cddr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(args[0]));
}

Val caddr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_cdr(args[0])));
}

Val caaddr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_car(get_cdr(get_cdr(args[0]))));
}

Val cdaddr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_car(get_cdr(get_cdr(args[0]))));
}

Val cdddr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_cdr(args[0])));
}

Val cadddr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_car(get_cdr(get_cdr(get_cdr(args[0]))));
}

Val cddddr_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    return get_cdr(get_cdr(get_cdr(get_cdr(args[0]))));
}

Val set_car_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type == TYPE_CONST_PAIR) {
        eprintf("Error: pair is immutable\n");
        exit(1);
    }
    if (args[0].type != TYPE_PAIR)
        type_error(args[0]);
    args[0].pair_data->car = args[1];
    return (Val){TYPE_VOID};
}

Val set_cdr_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type == TYPE_CONST_PAIR) {
        eprintf("Error: pair is immutable\n");
        exit(1);
    }
    if (args[0].type != TYPE_PAIR)
        type_error(args[0]);
    args[0].pair_data->cdr = args[1];
    return (Val){TYPE_VOID};
}
