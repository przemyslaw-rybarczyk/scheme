#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "io.h"
#include "../expr.h"
#include "assert.h"
#include "../display.h"
#include "../parser.h"
#include "../safemem.h"
#include "../insts.h"
#include "../compile.h"
#include "../exec.h"

struct val display_prim(struct val *args, int num) {
    args_assert(num == 1);
    inner_display_val(args[0]);
    return (struct val){TYPE_VOID};
}

struct val newline_prim(struct val *args, int num) {
    args_assert(num == 0);
    putchar('\n');
    return (struct val){TYPE_VOID};
}

struct val error_prim(struct val *args, int num) {
    if (num == 0) {
        fprintf(stderr, "Error.\n");
        exit(2);
    }
    fprintf(stderr, "Error: ");
    for (struct val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        inner_display_val(*arg_ptr);
        putchar(' ');
    }
    inner_display_val(args[num - 1]);
    putchar('\n');
    exit(2);
}

struct inst *read_prim(int num) {
    args_assert(num == 0);
    stack_pop();
    char c = getchar_nospace();
    if (c == EOF)
        exit(0);
    s_ungetc(c, stdin);
    struct sexpr *sexpr = parse();
    if (sexpr == NULL) {
        fprintf(stderr, "Syntax error: unexpected ')'\n");
        exit(1);
    }
    struct inst *eval = this_inst();
    compile_quote(sexpr);
    *next_inst() = (struct inst){INST_RETURN};
    return eval;
}
