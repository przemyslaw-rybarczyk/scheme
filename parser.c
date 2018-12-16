#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "safemem.h"
#include "expr.h"
#include "display.h"
#include "memory.h"
#include "symbol.h"
#include "env.h"
#include "exec.h"
#include "insts.h"

#define INIT_TOKEN_LENGTH 16

/* -- getchar_nospace
 * Gets the next character from stdin, skipping whitespace and comments.
 */
char getchar_nospace(void) {
    char c;
    while (1) {
        while (isspace(c = getchar()))
            ;
        if (c == ';') {
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            continue;
        }
        break;
    }
    return c;
}

Val get_token() {
    char c = getchar_nospace();

    // simple cases
    if (c == '(' || c == ')' || c == '\'' || c == EOF) {
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
        while ((s[i] = getchar()) != '"') {
            if (s[i] == EOF) {
                fprintf(stderr, "Syntax error: premature end of file - '\"' expected\n");
                exit(1);
            }
            i++;
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
    s[0] = c;
    while ((s[i] = getchar()) != EOF && !isspace(s[i])
            && s[i] != '(' && s[i] != ')' && s[i] != '\'' && s[i] != '"') {
        i++;
        if (i >= token_length) {
            token_length *= 2;
            s = s_realloc(s, token_length);
        }
    }
    s_ungetc(s[i], stdin);
    s[i] = '\0';

    // numeric literal
    if ('0' <= s[0] && s[0] <= '9' || (s[0] == '+' || s[0] == '-') && s[1] != '\0') {
        char *endptr;
        long long int_val = strtoll(s, &endptr, 10);
        if (*endptr == '\0')
            return (Val){TYPE_INT, {.int_data = int_val}};
        double float_val = strtod(s, &endptr);
        if (*endptr == '\0')
            return (Val){TYPE_FLOAT, {.float_data = float_val}};
        fprintf(stderr, "Syntax error: incorrect numeric literal %s\n", s);
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
        fprintf(stderr, "Syntax error: incorrect boolean or special literal %s\n", s);
        exit(1);
    }

    return (Val){TYPE_SYMBOL, {.string_data = intern_symbol(s)}};
}

/* -- read
 * Performs the entire parsing process.
 * Prevents syntax error on exit.
 */
int read_expr(void) {
    char c = getchar_nospace();
    if (c == EOF)
        return -1;
    s_ungetc(c, stdin);
    Global_env *exec_env = global_env;
    global_env = make_compile_env();
    int program = next_inst();
    insts[program] = (Inst){INST_EXPR};
    for (int program = compiler_pc; insts[program].type != INST_EOF; program = next_expr(program + 1))
        exec(program);
    exec(compile_pc);
    free(global_env);
    global_env = exec_env;
    return program;
}
