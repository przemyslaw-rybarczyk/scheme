#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "../types.h"
#include "../display.h"
#include "../env.h"
#include "../exec_stack.h"
#include "../insts.h"
#include "../parser.h"
#include "../safestd.h"
#include "assert.h"
#include "compiler.h"

Val display_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    display_val(args[0]);
    return (Val){TYPE_VOID};
}

Val newline_prim(Val *args, uint32_t num) {
    args_assert(num == 0);
    putchar('\n');
    return (Val){TYPE_VOID};
}

Val error_prim(Val *args, uint32_t num) {
    if (num == 0) {
        eprintf("Error.\n");
        exit(1);
    }
    eprintf("Error: ");
    for (Val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        display_val(*arg_ptr);
        putchar(' ');
    }
    display_val(args[num - 1]);
    putchar('\n');
    exit(1);
}

High_prim_return read_prim(Val *args, uint32_t num) {
    args_assert(num == 0);
    stack_pop();
    int c = fgetc_nospace(stdin);
    if (c == EOF32)
        exit(1);
    s_ungetc(c, stdin);
    compiler_input_file = stdin;
    return (High_prim_return){parse_pc, compiler_env};
}
