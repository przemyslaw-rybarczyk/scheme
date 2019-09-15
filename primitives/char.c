#include <stdio.h>
#include <stdlib.h>

#include "char.h"
#include "../types.h"
#include "../unicode/unicode.h"
#include "assert.h"

#define def_char_cmp_prim(name, op, fold) \
Val name(Val *args, uint32_t num) { \
    if (num == 0) \
        return true_val; \
    if (args[0].type != TYPE_CHAR) \
        type_error(args[0]); \
    for (Val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) { \
        if (arg_ptr[1].type != TYPE_CHAR) \
            type_error(arg_ptr[0]); \
        if (!(fold(arg_ptr[0].char_data) op fold(arg_ptr[1].char_data))) \
            return false_val; \
    } \
    return true_val; \
}

def_char_cmp_prim(char_eq_prim, ==, )
def_char_cmp_prim(char_lt_prim, <, )
def_char_cmp_prim(char_gt_prim, >, )
def_char_cmp_prim(char_le_prim, <=, )
def_char_cmp_prim(char_ge_prim, >=, )

def_char_cmp_prim(char_ci_eq_prim, ==, fold_case)
def_char_cmp_prim(char_ci_lt_prim, <, fold_case)
def_char_cmp_prim(char_ci_gt_prim, >, fold_case)
def_char_cmp_prim(char_ci_le_prim, <=, fold_case)
def_char_cmp_prim(char_ci_ge_prim, >=, fold_case)

#define def_char_prop_prim(name, prop) \
Val name(Val *args, uint32_t num) { \
    args_assert(num == 1); \
    if (args[0].type != TYPE_CHAR) \
        type_error(args[0]); \
    return (Val){TYPE_BOOL, {.int_data = prop(args[0].char_data)}}; \
}

def_char_prop_prim(char_alphabetic_prim, is_alphabetic)
def_char_prop_prim(char_numeric_prim, is_numeric)
def_char_prop_prim(char_whitespace_prim, is_whitespace)
def_char_prop_prim(char_upper_case_prim, is_uppercase)
def_char_prop_prim(char_lower_case_prim, is_lowercase)

Val char_to_integer_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_CHAR)
        type_error(args[0]);
    return (Val){TYPE_INT, {.int_data = args[0].char_data}};
}

Val integer_to_char_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    long long n = args[0].int_data;
    if (n < 0 || (n >= 0xD800 && n <= 0xDFFF) || n > 0x10FFFF) {
        eprintf("Error: 0x%04llX does not represent a valid Unicode character.\n", n);
        exit(1);
    }
    return (Val){TYPE_CHAR, {.char_data = (char32_t)n}};
}

Val char_upcase_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_CHAR)
        type_error(args[0]);
    return (Val){TYPE_CHAR, {.char_data = to_uppercase(args[0].char_data)}};
}

Val char_downcase_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_CHAR)
        type_error(args[0]);
    return (Val){TYPE_CHAR, {.char_data = to_lowercase(args[0].char_data)}};
}
