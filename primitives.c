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
#include "primitives/vector.h"
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

#define PRIM(name, internal_name) {{TYPE_PRIM, {.prim_data = internal_name}}, name}
#define H_PRIM(name, internal_name) {{TYPE_HIGH_PRIM, {.high_prim_data = internal_name}}, name}

static struct CString_binding cstring_r5rs_bindings[] = {
    PRIM("cons", cons_prim),
    PRIM("car", car_prim),
    PRIM("caar", caar_prim),
    PRIM("caaar", caaar_prim),
    PRIM("caaaar", caaaar_prim),
    PRIM("cdaaar", cdaaar_prim),
    PRIM("cdaar", cdaar_prim),
    PRIM("cadaar", cadaar_prim),
    PRIM("cddaar", cddaar_prim),
    PRIM("cdar", cdar_prim),
    PRIM("cadar", cadar_prim),
    PRIM("caadar", caadar_prim),
    PRIM("cdadar", cdadar_prim),
    PRIM("cddar", cddar_prim),
    PRIM("caddar", caddar_prim),
    PRIM("cdddar", cdddar_prim),
    PRIM("cdr", cdr_prim),
    PRIM("cadr", cadr_prim),
    PRIM("caadr", caadr_prim),
    PRIM("caaadr", caaadr_prim),
    PRIM("cdaadr", cdaadr_prim),
    PRIM("cdadr", cdadr_prim),
    PRIM("cadadr", cadadr_prim),
    PRIM("cddadr", cddadr_prim),
    PRIM("cddr", cddr_prim),
    PRIM("caddr", caddr_prim),
    PRIM("caaddr", caaddr_prim),
    PRIM("cdaddr", cdaddr_prim),
    PRIM("cdddr", cdddr_prim),
    PRIM("cadddr", cadddr_prim),
    PRIM("cddddr", cddddr_prim),
    PRIM("set-car!", set_car_prim),
    PRIM("set-cdr!", set_cdr_prim),
    PRIM("eq?", eq_prim),
    PRIM("eqv?", eqv_prim),
    PRIM("equal?", equal_prim),
    PRIM("pair?", pair_prim),
    PRIM("null?", null_prim),
    PRIM("number?", number_prim),
    PRIM("symbol?", symbol_prim),
    PRIM("procedure?", procedure_prim),
    PRIM("boolean?", boolean_prim),
    PRIM("char?", char_prim),
    PRIM("string?", string_q_prim),
    PRIM("vector?", vector_q_prim),
    PRIM("not", not_prim),
    PRIM("+", add_prim),
    PRIM("-", sub_prim),
    PRIM("*", mul_prim),
    PRIM("/", div_prim),
    PRIM("quotient", quotient_prim),
    PRIM("gcd", gcd_prim),
    PRIM("=", equ_prim),
    PRIM("<", lt_prim),
    PRIM(">", gt_prim),
    PRIM("make-rectangular", make_rectangular_prim),
    PRIM("list?", list_q_prim),
    PRIM("list", list_prim),
    PRIM("length", length_prim),
    PRIM("append", append_prim),
    PRIM("reverse", reverse_prim),
    PRIM("list-tail", list_tail_prim),
    PRIM("list-ref", list_ref_prim),
    PRIM("memq", memq_prim),
    PRIM("memv", memv_prim),
    PRIM("member", member_prim),
    PRIM("assq", assq_prim),
    PRIM("assv", assv_prim),
    PRIM("assoc", assoc_prim),
    H_PRIM("map", map_prim),
    H_PRIM("for-each", for_each_prim),
    H_PRIM("apply", apply_prim),
    PRIM("char=?", char_eq_prim),
    PRIM("char<?", char_lt_prim),
    PRIM("char>?", char_gt_prim),
    PRIM("char<=?", char_le_prim),
    PRIM("char>=?", char_ge_prim),
    PRIM("char-ci=?", char_ci_eq_prim),
    PRIM("char-ci<?", char_ci_lt_prim),
    PRIM("char-ci>?", char_ci_gt_prim),
    PRIM("char-ci<=?", char_ci_le_prim),
    PRIM("char-ci>=?", char_ci_ge_prim),
    PRIM("char-alphabetic?", char_alphabetic_prim),
    PRIM("char-numeric?", char_numeric_prim),
    PRIM("char-whitespace?", char_whitespace_prim),
    PRIM("char-upper-case?", char_upper_case_prim),
    PRIM("char-lower-case?", char_lower_case_prim),
    PRIM("char->integer", char_to_integer_prim),
    PRIM("integer->char", integer_to_char_prim),
    PRIM("char-upcase", char_upcase_prim),
    PRIM("char-downcase", char_downcase_prim),
    PRIM("make-string", make_string_prim),
    PRIM("string", string_prim),
    PRIM("string-length", string_length_prim),
    PRIM("string-ref", string_ref_prim),
    PRIM("string-set!", string_set_prim),
    PRIM("string=?", string_eq_prim),
    PRIM("string<?", string_lt_prim),
    PRIM("string>?", string_gt_prim),
    PRIM("string<=?", string_le_prim),
    PRIM("string>=?", string_ge_prim),
    PRIM("string-ci=?", string_ci_eq_prim),
    PRIM("string-ci<?", string_ci_lt_prim),
    PRIM("string-ci>?", string_ci_gt_prim),
    PRIM("string-ci<=?", string_ci_le_prim),
    PRIM("string-ci>=?", string_ci_ge_prim),
    PRIM("substring", substring_prim),
    PRIM("string-append", string_append_prim),
    PRIM("string->list", string_to_list_prim),
    PRIM("list->string", list_to_string_prim),
    PRIM("string-copy", string_copy_prim),
    PRIM("string-fill!", string_fill_prim),
    PRIM("symbol->string", symbol_to_string_prim),
    PRIM("string->symbol", string_to_symbol_prim),
    PRIM("make-vector", make_vector_prim),
    PRIM("vector", vector_prim),
    PRIM("vector-length", vector_length_prim),
    PRIM("vector-ref", vector_ref_prim),
    PRIM("vector-set!", vector_set_prim),
    PRIM("vector->list", vector_to_list_prim),
    PRIM("list->vector", list_to_vector_prim),
    PRIM("vector-fill!", vector_fill_prim),
    PRIM("display", display_prim),
    PRIM("newline", newline_prim),
    PRIM("error", error_prim),
    H_PRIM("read", read_prim),
};

uint32_t r5rs_bindings_size = sizeof(cstring_r5rs_bindings) / sizeof(struct CString_binding);

static struct CString_binding cstring_compiler_bindings[] = {
    PRIM("next-token", next_token_prim),
    PRIM("this-inst", this_inst_prim),
    PRIM("next-inst", next_inst_prim),
    PRIM("set-const!", set_const_prim),
    PRIM("set-var!", set_var_prim),
    PRIM("set-name!", set_name_prim),
    PRIM("set-def!", set_def_prim),
    PRIM("set-set!", set_set_prim),
    PRIM("set-set-name!", set_set_name_prim),
    PRIM("set-jump!", set_jump_prim),
    PRIM("set-jump-false!", set_jump_false_prim),
    PRIM("set-lambda!", set_lambda_prim),
    PRIM("set-call!", set_call_prim),
    PRIM("set-tail-call!", set_tail_call_prim),
    PRIM("set-return!", set_return_prim),
    PRIM("set-delete!", set_delete_prim),
    PRIM("set-cons!", set_cons_prim),
    PRIM("const-cons", const_cons_prim),
    PRIM("const-vector", const_vector_prim),
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
