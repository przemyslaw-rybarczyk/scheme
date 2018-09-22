#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "../expr.h"
#include "assert.h"
#include "../display.h"
#include "../parser.h"
#include "../eval.h"
#include "../safemem.h"

struct val display_prim(struct val_list *args) {
    assert_1_arg(args);
    inner_display_val(args->car);
    return (struct val){TYPE_VOID};
}

struct val newline_prim(struct val_list *args) {
    args_assert(args == NULL);
    putchar('\n');
    return (struct val){TYPE_VOID};
}

struct val error_prim(struct val_list *args) {
    if (args == NULL) {
        fprintf(stderr, "Error.\n");
        exit(2);
    }
    fprintf(stderr, "Error: ");
    while (args->cdr != NULL) {
        display_val(args->car);
        putchar(' ');
        args = args->cdr;
    }
    inner_display_val(args->car);
    putchar('\n');
    exit(2);
}

struct val read_prim(struct val_list *args) {
    char c = getchar_nospace();
    if (c == EOF)
        exit(0);
    s_ungetc(c, stdin);
    struct sexpr *sexpr = parse();
    if (sexpr == NULL) {
        fprintf(stderr, "Syntax error: unexpected ')'\n");
        exit(1);
    }
    return eval_quote(sexpr);
}
