// TODO remove ctype
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "types.h"
#include "env.h"
#include "exec.h"
#include "insts.h"
#include "memory.h"
#include "primitives/number.h"
#include "primitives/compiler.h"
#include "safestd.h"
#include "string.h"
#include "symbol.h"

#define INIT_TOKEN_LENGTH 16

/* -- fgetc_nospace
 * Gets the next character from a file, skipping whitespace and comments.
 */
int32_t fgetc32_nospace(FILE *f) {
    int32_t c;
    while (1) {
        // TODO replace with unicode check for space or control
        while (isspace(c = s_fgetc32(f)))
            ;
        if (c == ';') {
            while ((c = s_fgetc32(f)) != '\n' && c != EOF32)
                ;
            continue;
        }
        break;
    }
    return c;
}

int strbuf_eq_cstr(size_t len, char32_t *chars, char *s) {
    for (size_t i = 0; i < len; i++)
        if (chars[i] != s[i] || s[i] == '\0')
            return 0;
    return 1;
}

Val get_token(FILE *f) {
    int32_t c = fgetc32_nospace(f);

    // simple cases
    if (c == '(' || c == ')' || c == '.' || c == '\'' || c == EOF32) {
        Pair *pair = gc_alloc(sizeof(Pair));
        pair->car = (Val){TYPE_SYMBOL, {.string_data = intern_symbol(new_string_from_cstring("token"))}};
        pair->cdr = (Val){TYPE_INT, {.int_data = c}};
        return (Val){TYPE_PAIR, {.pair_data = pair}};
    }

    size_t capacity = INIT_TOKEN_LENGTH;
    char32_t *s = s_malloc(capacity * sizeof(char32_t));

    // string literal
    if (c == '"') {
        size_t i = 0;
        while ((c = s_fgetc32(f)) != '"') {
            if (c == EOF32) {
                eprintf("Syntax error: premature end of file - '\"' expected\n");
                exit(1);
            }
            s[i++] = (char32_t)c;
            if (i >= capacity) {
                capacity *= 2;
                s = s_realloc(s, capacity * sizeof(char32_t));
            }
        }
        return (Val){TYPE_STRING, {.string_data = new_gc_string(i, s)}};
    }

    // name
    size_t i = 1;
    s[0] = (char32_t)c;
    // TODO replace isspace
    while ((c = s_fgetc32(f)) != EOF32 && !isspace(c) && c != ';'
            && c != '(' && c != ')' && c != '\'' && c != '"') {
        s[i++] = (char32_t)c;
        if (i >= capacity) {
            capacity *= 2;
            s = s_realloc(s, capacity * sizeof(char32_t));
        }
    }
    s_ungetc(c, f);

    // numeric literal
    if (('0' <= s[0] && s[0] <= '9') || ((s[0] == '+' || s[0] == '-') && i > 1)) {
        char *num = s_malloc(i + 1);
        for (size_t j = 0; j < i; j++) {
            if (('0' <= s[j] && s[j] <= '9') || s[j] == '+' || s[j] == '-' || s[j] == '.')
                num[j] = (char)s[j];
            else
                goto invalid_num;
        }
        num[i] = '\0';
        char *endptr;
        long long int_val = strtoll(num, &endptr, 10);
        if (*endptr == '\0')
            return (Val){TYPE_INT, {.int_data = int_val}};
        double float_val = strtod(num, &endptr);
        if (*endptr == '\0')
            return (Val){TYPE_FLOAT, {.float_data = float_val}};
invalid_num:
        eprintf("Syntax error: incorrect numeric literal ");
        eputs32(new_string(i, s));
        eprintf("\n");
        exit(1);
    }

    // bool, char, or special
    if (s[0] == '#') {
        // char
        if (s[1] == '\\') {
            if (s[2] == '\0')
                return (Val){TYPE_CHAR, {.char_data = (unsigned char)s_fgetc(f)}};
            if (s[3] == '\0')
                return (Val){TYPE_CHAR, {.char_data = s[2]}};
            if (strbuf_eq_cstr(i, s, "#\\space"))
                return (Val){TYPE_CHAR, {.char_data = ' '}};
            if (strbuf_eq_cstr(i, s, "#\\newline"))
                return (Val){TYPE_CHAR, {.char_data = '\n'}};
        }
        if (strbuf_eq_cstr(i, s, "#f"))
            return (Val){TYPE_BOOL, {.int_data = 0}};
        if (strbuf_eq_cstr(i, s, "#t"))
            return (Val){TYPE_BOOL, {.int_data = 1}};
        if (strbuf_eq_cstr(i, s, "#!void"))
            return (Val){TYPE_VOID};
        if (strbuf_eq_cstr(i, s, "#!undef"))
            return (Val){TYPE_PRIM, {.prim_data = add_prim}};
        eprintf("Syntax error: incorrect literal ");
        eputs32(new_string(i, s));
        eprintf("\n");
        exit(1);
    }

    return (Val){TYPE_SYMBOL, {.string_data = intern_symbol(new_string(i, s))}};
}

uint32_t read_expr(FILE *f) {
    int c = fgetc32_nospace(f);
    if (c == EOF32)
        return UINT32_MAX;
    s_ungetc(c, f);
    compiler_input_file = f;
    uint32_t program = next_inst();
    insts[program] = (Inst){INST_EXPR};
    exec(compile_pc, compiler_env);
    return program;
}
