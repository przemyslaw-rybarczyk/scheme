#include <stdlib.h>

#include "expr.h"
#include "primitives/io.h"
#include "primitives/list.h"
#include "primitives/number.h"
#include "primitives/pair.h"
#include "primitives/predicates.h"

/* == primitives.c
 * This file specifies all primitives included in this Scheme implementation.
 * Their bindings are listed in the `prims` array.
 * They are listed roughly in order of usage frequency, to minimize lookup
 * times, since the primitives are added from the end of the array.
 * Similarily, the `high_prims` array contains all bindings of the high-order
 * primitives.
 * The actual functions of the primitives are defined in various files in the
 * `primitives` directory.
 */

// TODO REWRITE

struct prim_binding prims[] = {
    "cons", cons_prim,
    "car", car_prim,
    "cdr", cdr_prim,
    "null?", null_prim,
    "eq?", eq_prim,
    "=", equ_prim,
    "+", add_prim,
    "-", sub_prim,
    "*", mul_prim,
    "/", div_prim,
    "cadr", cadr_prim,
    "caddr", caddr_prim,
    "cadddr", cadddr_prim,
    "pair?", pair_prim,
    "not", not_prim,
    "<", lt_prim,
    ">", gt_prim,
    "list", list_prim,
    "cddr", cddr_prim,
    "cdddr", cdddr_prim,
    "cddddr", cddddr_prim,
    "equal?", equal_prim,
    "set-car!", set_car_prim,
    "set-cdr!", set_cdr_prim,
    "length", length_prim,
    "display", display_prim,
    "newline", newline_prim,
    "number?", number_prim,
    "symbol?", symbol_prim,
    "string?", string_prim,
    "procedure?", procedure_prim,
    "error", error_prim,
    "caar", caar_prim,
    "caaar", caaar_prim,
    "caaaar", caaaar_prim,
    "cdaaar", cdaaar_prim,
    "cdaar", cdaar_prim,
    "cadaar", cadaar_prim,
    "cddaar", cddaar_prim,
    "cdar", cdar_prim,
    "cadar", cadar_prim,
    "caadar", caadar_prim,
    "cdadar", cdadar_prim,
    "cddar", cddar_prim,
    "caddar", caddar_prim,
    "cdddar", cdddar_prim,
    "caadr", caadr_prim,
    "caaadr", caaadr_prim,
    "cdaadr", cdaadr_prim,
    "cdadr", cdadr_prim,
    "cadadr", cadadr_prim,
    "cddadr", cddadr_prim,
    "caaddr", caaddr_prim,
    "cdaddr", cdaddr_prim,
};

struct high_prim_binding high_prims[] = {
    "apply", apply_prim,
    "read", read_prim,
};

size_t prims_size = sizeof(prims) / sizeof(struct prim_binding);
size_t high_prims_size = sizeof(high_prims) / sizeof(struct high_prim_binding);
