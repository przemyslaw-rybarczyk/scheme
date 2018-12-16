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
#include "../exec.h"

Val display_prim(Val *args, int num) {
    args_assert(num == 1);
    inner_display_val(args[0]);
    return (Val){TYPE_VOID};
}

Val newline_prim(Val *args, int num) {
    args_assert(num == 0);
    putchar('\n');
    return (Val){TYPE_VOID};
}

Val error_prim(Val *args, int num) {
    if (num == 0) {
        fprintf(stderr, "Error.\n");
        exit(2);
    }
    fprintf(stderr, "Error: ");
    for (Val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        inner_display_val(*arg_ptr);
        putchar(' ');
    }
    inner_display_val(args[num - 1]);
    putchar('\n');
    exit(2);
}

int read_prim(int num) {
    //  TODO rewrite this function
//  args_assert(num == 0);
//  stack_pop();
//  char c = getchar_nospace();
//  if (c == EOF)
//      exit(0);
//  s_ungetc(c, stdin);
//  struct sexpr *sexpr = parse();
//  if (sexpr == NULL) {
//      fprintf(stderr, "Syntax error: unexpected ')'\n");
//      exit(1);
//  }
//  int eval = this_inst();
//  compile_quote(sexpr);
//  insts[next_inst()] = (Inst){INST_RETURN};
//  return eval;
}
