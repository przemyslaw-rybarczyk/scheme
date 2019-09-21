#include <stdio.h>
#include <stdlib.h>

#include "string.h"
#include "../types.h"
#include "../memory.h"
#include "../string.h"
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
