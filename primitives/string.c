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
