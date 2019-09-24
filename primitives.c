#include <stdlib.h>

#include "primitives.h"
#include "types.h"
#include "primitives/char.h"
#include "primitives/compiler.h"
#include "primitives/io.h"
#include "primitives/list.h"
#include "primitives/number.h"
#include "primitives/pair.h"
#include "primitives/predicates.h"
#include "primitives/string.h"
#include "safestd.h"
#include "string.h"

/* == primitives.c
 * This file specifies all primitives included in this Scheme implementation.
 * The actual definitions of the primitives are located in the `primitives` directory.
 */

struct CString_binding {
    Val val;
    char *var;
};

#define PRIM2(name, internal_name) {{TYPE_PRIM, {.prim_data = internal_name##_prim}}, name}
#define PRIM(name) PRIM2(#name, name)
#define PRIM_E(name) PRIM2(#name"!", name)
#define PRIM_Q(name) PRIM2(#name"?", name)
#define H_PRIM2(name, internal_name) {{TYPE_HIGH_PRIM, {.high_prim_data = internal_name##_prim}}, name}
#define H_PRIM(name) H_PRIM2(#name, name)

static struct CString_binding cstring_r5rs_bindings[] = {
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
    PRIM2("string?", string_q),
    PRIM_Q(procedure),
    PRIM_Q(boolean),
    PRIM_Q(char),
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
    H_PRIM(map),
    H_PRIM2("for-each", for_each),
    H_PRIM(apply),
    PRIM2("char=?", char_eq),
    PRIM2("char<?", char_lt),
    PRIM2("char>?", char_gt),
    PRIM2("char<=?", char_le),
    PRIM2("char>=?", char_ge),
    PRIM2("char-ci=?", char_ci_eq),
    PRIM2("char-ci<?", char_ci_lt),
    PRIM2("char-ci>?", char_ci_gt),
    PRIM2("char-ci<=?", char_ci_le),
    PRIM2("char-ci>=?", char_ci_ge),
    PRIM2("char-alphabetic?", char_alphabetic),
    PRIM2("char-numeric?", char_numeric),
    PRIM2("char-whitespace?", char_whitespace),
    PRIM2("char-upper-case?", char_upper_case),
    PRIM2("char-lower-case?", char_lower_case),
    PRIM2("char->integer", char_to_integer),
    PRIM2("integer->char", integer_to_char),
    PRIM2("char-upcase", char_upcase),
    PRIM2("char-downcase", char_downcase),
    PRIM2("make-string", make_string),
    PRIM(string),
    PRIM2("string-length", string_length),
    PRIM2("string-ref", string_ref),
    PRIM2("string-set!", string_set),
    PRIM2("string=?", string_eq),
    PRIM2("string<?", string_lt),
    PRIM2("string>?", string_gt),
    PRIM2("string<=?", string_le),
    PRIM2("string>=?", string_ge),
    PRIM2("string-ci=?", string_ci_eq),
    PRIM2("string-ci<?", string_ci_lt),
    PRIM2("string-ci>?", string_ci_gt),
    PRIM2("string-ci<=?", string_ci_le),
    PRIM2("string-ci>=?", string_ci_ge),
    PRIM(substring),
    PRIM2("string-append", string_append),
    PRIM2("string->list", string_to_list),
    PRIM2("list->string", list_to_string),
    PRIM2("string-copy", string_copy),
    PRIM2("string-fill!", string_fill),
    PRIM2("symbol->string", symbol_to_string),
    PRIM2("string->symbol", string_to_symbol),
    PRIM(display),
    PRIM(newline),
    PRIM(error),
    H_PRIM(read),
};

uint32_t r5rs_bindings_size = sizeof(cstring_r5rs_bindings) / sizeof(struct CString_binding);

static struct CString_binding cstring_compiler_bindings[] = {
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

uint32_t compiler_bindings_size = sizeof(cstring_compiler_bindings) / sizeof(struct CString_binding);

void setup_primitives(void) {
    r5rs_bindings = s_malloc(r5rs_bindings_size * sizeof(Binding));
    for (uint32_t i = 0; i < r5rs_bindings_size; i++)
        r5rs_bindings[i] = (Binding){cstring_r5rs_bindings[i].val,
            new_interned_string_from_cstring(cstring_r5rs_bindings[i].var)};
    compiler_bindings = s_malloc(compiler_bindings_size * sizeof(Binding));
    for (uint32_t i = 0; i < compiler_bindings_size; i++)
        compiler_bindings[i] = (Binding){cstring_compiler_bindings[i].val,
            new_interned_string_from_cstring(cstring_compiler_bindings[i].var)};
}
