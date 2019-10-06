#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "types.h"
#include "bigint/string.h"
#include "env.h"
#include "exec.h"
#include "insts.h"
#include "memory.h"
#include "primitives/number.h"
#include "primitives/compiler.h"
#include "safestd.h"
#include "string.h"
#include "unicode/unicode.h"

#define INIT_TOKEN_LENGTH 16

/* -- parser_buffer
 * A buffer necessary to provide one Unicode character of pushback.
 * A value of UINT32_MAX represents the buffer as empty.
 */
static char32_t parser_buffer = UINT32_MAX;

/* -- parser_fgetc32
 * Gets the next character from a file, using the parser buffer if it's full.
 */
int32_t parser_fgetc32(FILE *f) {
    if (parser_buffer != UINT32_MAX) {
        int32_t c = (int32_t)parser_buffer;
        parser_buffer = UINT32_MAX;
        return c;
    }
    return s_fgetc32(f);
}

/* -- parser_fgetc32_nospace
 * Gets the next character from a file, skipping whitespace and comments.
 * Uses the parser buffer.
 */
int32_t parser_fgetc32_nospace(FILE *f) {
    int32_t c;
    while (1) {
        while ((c = parser_fgetc32(f)) != EOF32 && (is_whitespace((char32_t)c) || is_control((char32_t)c)))
            ;
        if (c == ';') {
            while ((c = parser_fgetc32(f)) != '\n' && c != EOF32)
                ;
            continue;
        }
        break;
    }
    return c;
}

/* -- parser_init
 * Initializes the parser by and checking for EOF.
 * If there are non-whitespace, non-comment characters remaining, it returns 1
 * and puts the character back into the parser_buffer.
 * If EOF is reached instead, it returns 0.
 * This function should always be called before using the compiler.
 */
int parser_init(FILE *f) {
    int32_t c = parser_fgetc32_nospace(f);
    if (c == EOF32)
        return 0;
    parser_buffer = (char32_t)c;
    return 1;
}

int strbuf_eq_cstr(size_t len, char32_t *chars, char *s) {
    for (size_t i = 0; i < len; i++)
        if (chars[i] != s[i] || s[i] == '\0')
            return 0;
    return s[len] == '\0';
}

static String *new_string(size_t len, char32_t *chars) {
    String *str = s_malloc(sizeof(String) + len * sizeof(char32_t));
    str->len = len;
    memcpy(str->chars, chars, len * sizeof(char32_t));
    free(chars);
    return str;
}

static String *new_gc_string(size_t len, char32_t *chars) {
    String *str = gc_alloc_string(len);
    memcpy(str->chars, chars, len * sizeof(char32_t));
    free(chars);
    return str;
}

/* -- get_token
 * Returns value of the literal for literal tokens, symbols for variable names,
 * the pair (token . <ASCII code>) for characters '(', ')', '\'', '.' and '#'.
 * Note that '.' and '#' are not considered separate tokens in all contexts.
 */
Val get_token(FILE *f) {
    int32_t c = parser_fgetc32_nospace(f);

    if (c == EOF32) {
        eprintf("Error: unexpected end of file\n");
        exit(1);
    }

    // simple cases
    if (c == '(' || c == ')' || c == '\'') {
        Pair *pair = gc_alloc(sizeof(Pair));
        pair->car = (Val){TYPE_SYMBOL, {.string_data = new_interned_string_from_cstring("token")}};
        pair->cdr = (Val){TYPE_INT, {.int_data = c}};
        return (Val){TYPE_PAIR, {.pair_data = pair}};
    }

    size_t capacity = INIT_TOKEN_LENGTH;
    char32_t *s = s_malloc(capacity * sizeof(char32_t));

    // string literal
    if (c == '"') {
        size_t i = 0;
        while ((c = parser_fgetc32(f)) != '"') {
            if (c == EOF32) {
                eprintf("Syntax error: premature end of file - '\"' expected\n");
                exit(1);
            }
            if (c == '\\') {
                c = parser_fgetc32(f);
                if (c == EOF32) {
                    eprintf("Syntax error: premature end of file - '\"' expected\n");
                    exit(1);
                }
                if (c != '"' && c != '\\') {
                    eprintf("Syntax error: invalid escape sequence in string: \\");
                    fputc32((char32_t)c, stderr);
                    eprintf("\n");
                    exit(1);
                }
            }
            s[i++] = (char32_t)c;
            if (i >= capacity) {
                capacity *= 2;
                s = s_realloc(s, capacity * sizeof(char32_t));
            }
        }
        return (Val){TYPE_CONST_STRING, {.string_data = new_string(i, s)}};
    }

    // name
    size_t i = 1;
    s[0] = (char32_t)c;
    while ((c = parser_fgetc32(f)) != EOF32 && !(is_whitespace((char32_t)c) || is_control((char32_t)c))
            && c != ';' && c != '(' && c != ')' && c != '\'' && c != '"') {
        s[i++] = (char32_t)c;
        if (i >= capacity) {
            capacity *= 2;
            s = s_realloc(s, capacity * sizeof(char32_t));
        }
    }
    parser_buffer = (char32_t)c;

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
        if (*endptr == '\0') {
            free(s);
            free(num);
            return (Val){TYPE_INT, {.int_data = int_val}};
        }
        double float_val = strtod(num, &endptr);
        if (*endptr == '\0') {
            free(s);
            free(num);
            return (Val){TYPE_FLOAT, {.float_data = float_val}};
        }
invalid_num:
        free(num);
        eprintf("Syntax error: incorrect numeric literal ");
        eputs32(new_gc_string(i, s));
        eprintf("\n");
        exit(1);
    }

    if (i == 1 && (s[0] == '.' || s[0] == '#')) {
        Pair *pair = gc_alloc(sizeof(Pair));
        pair->car = (Val){TYPE_SYMBOL, {.string_data = new_interned_string_from_cstring("token")}};
        pair->cdr = (Val){TYPE_INT, {.int_data = s[0]}};
        free(s);
        return (Val){TYPE_PAIR, {.pair_data = pair}};
    }

    // bool, char, or special
    if (s[0] == '#') {
        // char
        if (s[1] == '\\') {
            if (i == 2) {
                char32_t c = parser_buffer;
                parser_buffer = UINT32_MAX;
                free(s);
                return (Val){TYPE_CHAR, {.char_data = c}};
            } if (i == 3) {
                char32_t c = s[2];
                free(s);
                return (Val){TYPE_CHAR, {.char_data = c}};
            } if (strbuf_eq_cstr(i, s, "#\\space")) {
                free(s);
                return (Val){TYPE_CHAR, {.char_data = ' '}};
            } if (strbuf_eq_cstr(i, s, "#\\newline")) {
                free(s);
                return (Val){TYPE_CHAR, {.char_data = '\n'}};
            }
        } if (s[1] == 'x') {
            Bigint *bi = read_bigint_hexadecimal(i - 2, s + 2);
            if (bi == NULL) {
                eprintf("Error: invalid hexadecimal literal ");
                eputs32(new_gc_string(i, s));
                eprintf("\n");
                exit(1);
            }
            return (Val){TYPE_BIGINT, {.bigint_data = bi}};
        } if (strbuf_eq_cstr(i, s, "#f")) {
            free(s);
            return (Val){TYPE_BOOL, {.int_data = 0}};
        } if (strbuf_eq_cstr(i, s, "#t")) {
            free(s);
            return (Val){TYPE_BOOL, {.int_data = 1}};
        } if (strbuf_eq_cstr(i, s, "#!void")) {
            free(s);
            return (Val){TYPE_VOID};
        } if (strbuf_eq_cstr(i, s, "#!undef")) {
            free(s);
            return (Val){TYPE_UNDEF};
        }
        eprintf("Syntax error: incorrect literal ");
        eputs32(new_gc_string(i, s));
        eprintf("\n");
        exit(1);
    }

    String *str = new_interned_string(i, s);
    free(s);
    return (Val){TYPE_SYMBOL, {.string_data = str}};
}

uint32_t read_expr(FILE *f) {
    if (!parser_init(f))
        return UINT32_MAX;
    compiler_input_file = f;
    uint32_t program = next_inst();
    insts[program] = (Inst){INST_EXPR};
    exec(compile_pc, compiler_env);
    return program;
}
