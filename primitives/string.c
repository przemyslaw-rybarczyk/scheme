#include <stdio.h>
#include <stdlib.h>

#include "string.h"
#include "../types.h"
#include "../memory.h"
#include "../string.h"
#include "../unicode/unicode.h"
#include "assert.h"

Val make_string_prim(Val *args, uint32_t num) {
    args_assert(num == 1 || num == 2);
    char32_t c = 0;
    if (num == 2) {
        if (args[1].type != TYPE_CHAR)
            type_error(args[1]);
        c = args[1].char_data;
    }
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[0].int_data < 0) {
        eprintf("Error: negative length of string provided\n");
        exit(1);
    }
    String *str = gc_alloc_string((size_t)args[0].int_data);
    for (size_t i = 0; i < args[0].int_data; i++)
        str->chars[i] = c;
    return (Val){TYPE_STRING, {.string_data = str}};
}

Val string_prim(Val *args, uint32_t num) {
    String *str = gc_alloc_string(num);
    for (uint32_t i = 0; i < num; i++) {
        if (args[i].type != TYPE_CHAR)
            type_error(args[i]);
        str->chars[i] = args[i].char_data;
    }
    return (Val){TYPE_STRING, {.string_data = str}};
}

Val string_length_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_STRING && args[0].type != TYPE_CONST_STRING)
        type_error(args[0]);
    return (Val){TYPE_INT, {.int_data = (long long)args[0].string_data->len}};
}

Val string_ref_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_STRING && args[0].type != TYPE_CONST_STRING)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    if (args[1].int_data < 0 || args[1].int_data >= args[0].string_data->len) {
        eprintf("Error: string index out of range\n");
        exit(1);
    }
    return (Val){TYPE_CHAR, {.char_data = args[0].string_data->chars[args[1].int_data]}};
}

Val string_set_prim(Val *args, uint32_t num) {
    args_assert(num == 3);
    if (args[0].type == TYPE_CONST_STRING) {
        eprintf("Error: string is immutable\n");
        exit(1);
    }
    if (args[0].type != TYPE_STRING)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    if (args[2].type != TYPE_CHAR)
        type_error(args[2]);
    if (args[1].int_data < 0 || args[1].int_data >= args[0].string_data->len) {
        eprintf("Error: string index out of range\n");
        exit(1);
    }
    args[0].string_data->chars[args[1].int_data] = args[2].char_data;
    return (Val){TYPE_VOID};
}

ssize_t string_cmp(String *str1, String *str2) {
    size_t len = (str1->len < str2->len) ? str1->len : str2->len;
    for (size_t i = 0; i < len; i++) {
        if (str1->chars[i] != str2->chars[i])
            return (ssize_t)str1->chars[i] - (ssize_t)str2->chars[i];
    }
    return (ssize_t)str1->len - (ssize_t)str2->len;
}

ssize_t string_ci_cmp(String *str1, String *str2) {
    if (str1->len == 0 || str2->len == 0)
        return (ssize_t)str1->len - (ssize_t)str2->len;
    String *str[2] = {str1, str2};
    size_t i[2] = {0, 0};
    uint16_t *fold[2] = {full_foldcase(str1->chars[0]), full_foldcase(str2->chars[0])};
    int fold_i[2] = {0, 0};
    int loop[2] = {1, 1};
    while (loop[0] && loop[1]) {
        char32_t c[2];
        for (int j = 0; j < 2; j++) {
            if (fold[j] == NULL) {
                c[j] = fold_case(str[j]->chars[i[j]++]);
                if (i[j] == str[j]->len)
                    loop[j] = 0;
                else
                    fold[j] = full_foldcase(str[j]->chars[i[j]]);
            } else {
                c[j] = fold[j][fold_i[j]++];
                if (fold_i[j] == MAX_FULL_CASING_LENGTH || fold[j][fold_i[j]] == 0) {
                    i[j]++;
                    fold_i[j] = 0;
                    if (i[j] == str[j]->len)
                        loop[j] = 0;
                    else
                        fold[j] = full_foldcase(str[j]->chars[i[j]]);
                }
            }
        }
        if (c[0] != c[1]) {
            return (int32_t)c[0] - (int32_t)c[1];
        }
    }
    return loop[0] - loop[1];
}

#define def_string_cmp_prim(name, cmp, op) \
Val name(Val *args, uint32_t num) { \
    if (num == 0) \
        return true_val; \
    if (args[0].type != TYPE_STRING && args[0].type != TYPE_CONST_STRING) \
        type_error(args[0]); \
    for (Val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) { \
        if (arg_ptr[1].type != TYPE_STRING && arg_ptr[1].type != TYPE_CONST_STRING) \
            type_error(arg_ptr[1]); \
        if (!(cmp(arg_ptr[0].string_data, arg_ptr[1].string_data) op 0)) \
            return false_val; \
    } \
    return true_val; \
}

def_string_cmp_prim(string_eq_prim, string_eq, !=);
def_string_cmp_prim(string_lt_prim, string_cmp, <);
def_string_cmp_prim(string_gt_prim, string_cmp, >);
def_string_cmp_prim(string_le_prim, string_cmp, <=);
def_string_cmp_prim(string_ge_prim, string_cmp, >=);
def_string_cmp_prim(string_ci_eq_prim, string_ci_cmp, ==);
def_string_cmp_prim(string_ci_lt_prim, string_ci_cmp, <);
def_string_cmp_prim(string_ci_gt_prim, string_ci_cmp, >);
def_string_cmp_prim(string_ci_le_prim, string_ci_cmp, <=);
def_string_cmp_prim(string_ci_ge_prim, string_ci_cmp, >=);
