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

void utf8_error() {
    eprintf("Error: invalid UTF-8 input\n");
    exit(1);
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

    // bool, char, or special
    if (s[0] == '#') {
        // char
        if (s[1] == '\\') {
            if (s[2] == '\0')
                return (Val){TYPE_CHAR, {.char_data = (unsigned char)s_fgetc(f)}};
            char *ch = &s[2];
            int bytes;
            char32_t c = (unsigned char)ch[0];
            if ((ch[0] & 0x80) == 0x00) {
                bytes = 1;
            } else {
                if ((ch[1] & 0xC0) != 0x80)
                    utf8_error();
                c = (c << 6) | (ch[1] & 0x3F);
                if ((ch[0] & 0xE0) == 0xC0) {
                    c &= 0x7FF;
                    bytes = 2;
                } else {
                    if ((ch[2] & 0xC0) != 0x80)
                        utf8_error();
                    c = (c << 6) | (ch[2] & 0x3F);
                    if ((ch[0] & 0xF0) == 0xE0) {
                        c &= 0xFFFF;
                        bytes = 3;
                    } else {
                        if ((ch[3] & 0xC0) != 0x80)
                            utf8_error();
                        c = (c << 6) | (ch[3] & 0x3F);
                        if ((ch[0] & 0xF8) == 0xF0) {
                            c &= 0x1FFFFF;
                            bytes = 4;
                        } else {
                            utf8_error();
                        }
                    }
                }
            }
            if (ch[bytes] == '\0')
                return (Val){TYPE_CHAR, {.char_data = c}};
            if (strcmp(s, "#\\space") == 0)
                return (Val){TYPE_CHAR, {.char_data = ' '}};
            if (strcmp(s, "#\\newline") == 0)
                return (Val){TYPE_CHAR, {.char_data = '\n'}};
        }
        if (strcmp(s, "#f") == 0)
            return (Val){TYPE_BOOL, {.int_data = 0}};
        if (strcmp(s, "#t") == 0)
            return (Val){TYPE_BOOL, {.int_data = 1}};
        if (strcmp(s, "#!void") == 0)
            return (Val){TYPE_VOID};
        if (strcmp(s, "#!undef") == 0)
            return (Val){TYPE_PRIM, {.prim_data = add_prim}};
        eprintf("Syntax error: incorrect literal %s\n", s);
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
