#include <stdlib.h>

#include "types.h"
#include "primitives/compiler.h"
#include "primitives/io.h"
#include "primitives/list.h"
#include "primitives/number.h"
#include "primitives/pair.h"
#include "primitives/predicates.h"

/* == primitives.c
 * This file specifies all primitives included in this Scheme implementation.
 * The actual definitions of the primitives are located in the `primitives` directory.
 */

#define PRIM2(name, internal_name) {{TYPE_PRIM, {.prim_data = internal_name##_prim}}, name}
#define PRIM(name) PRIM2(#name, name)
#define PRIM_E(name) PRIM2(#name"!", name)
#define PRIM_Q(name) PRIM2(#name"?", name)
#define H_PRIM(name) {{TYPE_HIGH_PRIM, {.high_prim_data = name##_prim}}, #name}

Binding r5rs_bindings[] = {
    PRIM(cons),
    PRIM(car),
    PRIM(caar),
    PRIM(caaar),
    PRIM(caaaar),
    PRIM(cdaaar),
    PRIM(cdaar),
    PRIM(cadaar),
    PRIM(cddaar),
    PRIM(cdar),
    PRIM(cadar),
    PRIM(caadar),
    PRIM(cdadar),
    PRIM(cddar),
    PRIM(caddar),
    PRIM(cdddar),
    PRIM(cdr),
    PRIM(cadr),
    PRIM(caadr),
    PRIM(caaadr),
    PRIM(cdaadr),
    PRIM(cdadr),
    PRIM(cadadr),
    PRIM(cddadr),
    PRIM(cddr),
    PRIM(caddr),
    PRIM(caaddr),
    PRIM(cdaddr),
    PRIM(cdddr),
    PRIM(cadddr),
    PRIM(cddddr),
    PRIM2("set-car!", set_car),
    PRIM2("set-cdr!", set_cdr),
    PRIM_Q(eq),
    PRIM_Q(eqv),
    PRIM_Q(equal),
    PRIM_Q(pair),
    PRIM_Q(null),
    PRIM_Q(number),
    PRIM_Q(symbol),
    PRIM_Q(string),
    PRIM_Q(procedure),
    PRIM_Q(boolean),
    PRIM(not),
    PRIM2("+", add),
    PRIM2("-", sub),
    PRIM2("*", mul),
    PRIM2("/", div),
    PRIM2("=", equ),
    PRIM2("<", lt),
    PRIM2(">", gt),
    PRIM2("list?", list_q),
    PRIM(list),
    PRIM(length),
    PRIM(append),
    PRIM(reverse),
    PRIM2("list-tail", list_tail),
    PRIM2("list-ref", list_ref),
    PRIM(memq),
    PRIM(memv),
    PRIM(member),
    PRIM(assq),
    PRIM(assv),
    PRIM(assoc),
    H_PRIM(apply),
    PRIM(display),
    PRIM(newline),
    PRIM(error),
    H_PRIM(read),
};

uint32_t r5rs_bindings_size = sizeof(r5rs_bindings) / sizeof(Binding);

Binding compiler_bindings[] = {
    PRIM2("next-token", next_token),
    PRIM2("this-inst", this_inst),
    PRIM2("next-inst", next_inst),
    PRIM2("set-const!", set_const),
    PRIM2("set-var!", set_var),
    PRIM2("set-name!", set_name),
    PRIM2("set-def!", set_def),
    PRIM2("set-set!", set_set),
    PRIM2("set-set-name!", set_set_name),
    PRIM2("set-jump!", set_jump),
    PRIM2("set-jump-false!", set_jump_false),
    PRIM2("set-lambda!", set_lambda),
    PRIM2("set-call!", set_call),
    PRIM2("set-tail-call!", set_tail_call),
    PRIM2("set-return!", set_return),
    PRIM2("set-delete!", set_delete),
    PRIM2("set-cons!", set_cons),
    PRIM2("new-symbol", new_symbol),
};

uint32_t compiler_bindings_size = sizeof(compiler_bindings) / sizeof(Binding);
