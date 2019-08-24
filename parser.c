#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "types.h"
#include "env.h"
#include "exec.h"
#include "insts.h"
#include "memory.h"
#include "primitives/number.h"
#include "primitives/compiler.h"
#include "safestd.h"
#include "symbol.h"

#define INIT_TOKEN_LENGTH 16

/* -- fgetc_nospace
 * Gets the next character from a file, skipping whitespace and comments.
 */
int fgetc_nospace(FILE *f) {
    int c;
    while (1) {
        while (isspace(c = s_fgetc(f)))
            ;
        if (c == ';') {
            while ((c = s_fgetc(f)) != '\n' && c != EOF)
                ;
            continue;
        }
        break;
    }
    return c;
}

Val get_token(FILE *f) {
    int c = fgetc_nospace(f);

    // simple cases
    if (c == '(' || c == ')' || c == '.' || c == '\'' || c == EOF) {
        Pair *pair = gc_alloc(sizeof(Pair));
        pair->car = (Val){TYPE_SYMBOL, {.string_data = intern_symbol("token")}};
        pair->cdr = (Val){TYPE_INT, {.int_data = c}};
        return (Val){TYPE_PAIR, {.pair_data = pair}};
    }

    size_t token_length = INIT_TOKEN_LENGTH;
    char *s = s_malloc(token_length);

    // string literal
    if (c == '"') {
        size_t i = 0;
        while ((c = s_fgetc(f)) != '"') {
            if (c == EOF) {
                eprintf("Syntax error: premature end of file - '\"' expected\n");
                exit(1);
            }
            s[i++] = (char)c;
            if (i >= token_length) {
                token_length *= 2;
                s = s_realloc(s, token_length);
            }
        }
        s[i] = '\0';
        return (Val){TYPE_STRING, {.string_data = s}};
    }

    // name
    size_t i = 1;
    s[0] = (char)c;
    while ((c = s_fgetc(f)) != EOF && !isspace(c) && c != ';'
            && c != '(' && c != ')' && c != '\'' && c != '"') {
        s[i++] = (char)c;
        if (i >= token_length) {
            token_length *= 2;
            s = s_realloc(s, token_length);
        }
    }
    s_ungetc(c, f);
    s[i] = '\0';

    // numeric literal
    if (('0' <= s[0] && s[0] <= '9') || ((s[0] == '+' || s[0] == '-') && s[1] != '\0')) {
        char *endptr;
        long long int_val = strtoll(s, &endptr, 10);
        if (*endptr == '\0')
            return (Val){TYPE_INT, {.int_data = int_val}};
        double float_val = strtod(s, &endptr);
        if (*endptr == '\0')
            return (Val){TYPE_FLOAT, {.float_data = float_val}};
        eprintf("Syntax error: incorrect numeric literal %s\n", s);
        exit(1);
    }

    // bool or special
    if (s[0] == '#') {
        if (strcmp(s, "#f") == 0)
            return (Val){TYPE_BOOL, {.int_data = 0}};
        if (strcmp(s, "#t") == 0)
            return (Val){TYPE_BOOL, {.int_data = 1}};
        if (strcmp(s, "#!void") == 0)
            return (Val){TYPE_VOID};
        if (strcmp(s, "#!undef") == 0)
            return (Val){TYPE_PRIM, {.prim_data = add_prim}};
        eprintf("Syntax error: incorrect boolean or special literal %s\n", s);
        exit(1);
    }

    return (Val){TYPE_SYMBOL, {.string_data = intern_symbol(s)}};
}

uint32_t read_expr(FILE *f) {
    int c = fgetc_nospace(f);
    if (c == EOF)
        return UINT32_MAX;
    s_ungetc(c, f);
    compiler_input_file = f;
    uint32_t program = next_inst();
    insts[program] = (Inst){INST_EXPR};
    exec(compile_pc, compiler_env);
    return program;
}
