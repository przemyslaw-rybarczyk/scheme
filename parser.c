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

#define INIT_TOKEN_LENGTH 16
#define NAME_TOKEN 'A'
#define STRING_TOKEN '"'

void syntax_assert(int assertion, const char *tag, struct sexpr *sexpr) {
    if (assertion)
        return;
    fprintf(stderr, "Syntax error: badly formed %s expression:\n", tag);
    display_sexpr(sexpr);
    putchar('\n');
    exit(1);
}

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

/* -- get_token
 * Returns the type of the next token.
 * For (, ), ' it returns the character.
 * For names and string literals it returns NAME_TOKEN or STRING_TOKEN
 * appropriately and stores the name or string contents in the string
 * referenced by s_ptr.
 * String literals ignore escape sequences and newline.
 * It ignores comments from the first semicolon in a given line until
 * its end.
 * Returns EOF on end of file.
 */
char get_token(char **s_ptr) {
    char c = getchar_nospace();

    // simple cases
    if (c == '(' || c == ')' || c == '\'' || c == EOF)
        return c;

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
        *s_ptr = s;
        return STRING_TOKEN;
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
    *s_ptr = s;
    return NAME_TOKEN;
}

struct sexpr_list *parse_list();

/* -- parse
 * Parses the first expression into an intermediate syntax tree.
 * Recognizes number, bool, and string literals.
 * Converts '... into (quote ...)
 * Returns NULL in case of a closed parenthesis without returning it to stdin.
 */
struct sexpr *parse(void) {
    char *text;
    char type = get_token(&text);
    if (type == ')')
        return NULL;
    struct sexpr *sexpr = s_malloc(sizeof(struct sexpr));
    switch (type) {

    case '(':
        sexpr->type = SEXPR_CONS;
        sexpr->data.cons = parse_list();
        return sexpr;

    case '\'': {
        struct sexpr *quoted = parse();
        if (quoted == NULL) {
            fprintf(stderr, "Syntax error: incorrect quotation\n");
            exit(1);
        }
        sexpr->type = SEXPR_CONS;
        struct sexpr_list *cons = s_malloc(sizeof(struct sexpr_list));
        struct sexpr *car_sexpr = s_malloc(sizeof(struct sexpr));
        car_sexpr->type = SEXPR_ATOM;
        car_sexpr->data.atom = "quote";
        struct sexpr_list *cdr_pair = s_malloc(sizeof(struct sexpr_list));
        cdr_pair->car = quoted;
        cdr_pair->cdr = NULL;
        cons->car = car_sexpr;
        cons->cdr = cdr_pair;
        sexpr->data.cons = cons;
        return sexpr;
    }

    case NAME_TOKEN:
        // number
        if ('0' <= text[0] && text[0] <= '9'
                || (text[0] == '-' || text[0] == '+') && '0' <= text[1] && text[1] <= '9') {
            sexpr->type = SEXPR_LITERAL;
            char *endptr;
            long long int_val = strtoll(text, &endptr, 10);

            // integer
            if (*endptr == '\0') {
                sexpr->data.literal.type = TYPE_INT;
                sexpr->data.literal.data.int_data = int_val;
                return sexpr;
            }

            // floating point
            double float_val = strtod(text, &endptr);
            if (*endptr == '\0') {
                sexpr->data.literal.type = TYPE_FLOAT;
                sexpr->data.literal.data.float_data = float_val;
                return sexpr;
            }

            fprintf(stderr, "Syntax error: incorrect numeric literal %s\n", text);
            exit(1);
        }

        // boolean
        if (text[0] == '#') {
            sexpr->type = SEXPR_LITERAL;
            sexpr->data.literal.type = TYPE_BOOL;
            switch (text[1]) {
            case 'f':
                sexpr->data.literal.data.int_data = 0;
                return sexpr;
            case 't':
                sexpr->data.literal.data.int_data = 1;
                return sexpr;
            }
            fprintf(stderr, "Syntax error: incorrect boolean literal %s\n", text);
            exit(1);
        }

        // atom
        sexpr->type = SEXPR_ATOM;
        sexpr->data.atom = text;
        return sexpr;

    case STRING_TOKEN:
        sexpr->type = SEXPR_LITERAL;
        sexpr->data.literal.type = TYPE_STRING;
        sexpr->data.literal.data.string_data = text;
        return sexpr;

    case EOF:
        fprintf(stderr, "Syntax error: premature end of file - expected ')'\n");
        exit(1);
    }
}

struct sexpr_list *parse_list(void) {
    struct sexpr *car = parse();
    if (car != NULL) {
        struct sexpr_list *pair = s_malloc(sizeof(struct sexpr_list));
        pair->car = car;
        pair->cdr = parse_list();
        return pair;
    }
    return NULL;
}

struct expr_list *analyze_list(struct sexpr_list *list, struct name_env *env);
struct name_list *analyze_param_list(struct sexpr_list *list, const char* tag, struct sexpr *sexpr);
struct expr *analyze_seq_expr(struct sexpr_list *seq, struct name_env *env);
struct expr *analyze_cond_clauses(struct sexpr_list *clauses, struct sexpr *cond, struct name_env *env);
struct expr *analyze_or_args(struct sexpr_list *args, struct name_env *env);

const char *keywords[] = {"and", "begin", "cond", "define", "else", "if", "lambda", "let", "set!", "or", "quote"};

void assert_not_keyword(const char *name) {
    for (const char** keyword = keywords;
            keyword < keywords + sizeof(keywords) / sizeof(const char *);
            keyword++) {
        if (strcmp(name, *keyword) == 0) {
            fprintf(stderr,  "Syntax error: variable name %s is a keyword\n", name);
            exit(1);
        }
    }
}

/* -- lookup_name
 * TODO explain
 */
struct env_loc lookup_name(char *name, struct name_env *env) {
    int frame = 0;
    for (struct name_env *env_part = env; env_part != NULL; env_part = env_part->next) {
        int index = 0;
        for (struct name_list *frame_part = env_part->frame; frame_part != NULL; frame_part = frame_part->cdr) {
            if (strcmp(name, frame_part->car) == 0)
                return (struct env_loc){frame, index};
            index++;
        }
        frame++;
    }
    return (struct env_loc){-1, -1};
}

/* -- analyze
 * Analyzes a parsed expression.
 * Recognizes special forms and turns them into specific constructs.
 * Prevents invalid syntax for special forms and using keywords as variables.
 * Converts cond, let, and, or expressions into compounds of simpler expressions.
 */
struct expr* analyze(struct sexpr* sexpr, struct name_env *env) {
    struct expr* expr = s_malloc(sizeof(struct expr));
    if (sexpr == NULL) {
        fprintf(stderr, "Syntax error: unexpected ')'\n");
        exit(1);
    }
    switch (sexpr->type) {

    case SEXPR_LITERAL:
        expr->type = EXPR_LITERAL;
        expr->data.literal = sexpr->data.literal;
        return expr;

    case SEXPR_ATOM:
        assert_not_keyword(sexpr->data.atom);
        struct env_loc var = lookup_name(sexpr->data.atom, env);
        if (var.frame != -1) {
            expr->type = EXPR_VAR;
            expr->data.var = var;
        } else {
            expr->type = EXPR_NAME;
            expr->data.name = sexpr->data.atom;
        }
        return expr;

    case SEXPR_CONS:
        if (sexpr->data.cons == NULL) {
            fprintf(stderr, "Syntax error: () is not a valid expression\n");
            exit(1);
        }
        if (sexpr->data.cons->car->type == SEXPR_ATOM) {
            const char *tag = sexpr->data.cons->car->data.atom;
            struct sexpr_list *cdr = sexpr->data.cons->cdr;

            if (strcmp(tag, "define") == 0) {
                expr->type = EXPR_DEF;
                syntax_assert(cdr != NULL, tag, sexpr);
                syntax_assert(cdr->car->type != SEXPR_LITERAL, tag, sexpr);
                switch (cdr->car->type) {
                case SEXPR_ATOM:
                    syntax_assert(cdr->cdr != NULL, tag, sexpr);
                    syntax_assert(cdr->cdr->cdr == NULL, tag, sexpr);
                    expr->data.name_binding.var = cdr->car->data.atom;
                    expr->data.name_binding.val = analyze(cdr->cdr->car, env);
                    return expr;
                case SEXPR_CONS:
                    // lambda definition
                    syntax_assert(cdr->car->data.cons != NULL, tag, sexpr);
                    syntax_assert(cdr->car->data.cons->car->type == SEXPR_ATOM, tag, sexpr);
                    expr->data.name_binding.var = cdr->car->data.cons->car->data.atom;
                    struct expr *lambda = s_malloc(sizeof(struct expr));
                    lambda->type = EXPR_LAMBDA;
                    struct name_list *params = analyze_param_list(cdr->car->data.cons->cdr, tag, sexpr);
                    int len = 0;
                    for (struct name_list *params_part = params; params_part != NULL; params_part = params_part->cdr)
                        len++;
                    lambda->data.lambda.params = len;
                    struct name_env *lambda_env = s_malloc(sizeof(struct name_env));
                    lambda_env->frame = params;
                    lambda_env->next = env;
                    lambda->data.lambda.body = analyze_list(cdr->cdr, lambda_env);
                    expr->data.name_binding.val = lambda;
                    return expr;
                }
            }

            if (strcmp(tag, "set!") == 0) {
                syntax_assert(cdr != NULL, tag, sexpr);
                syntax_assert(cdr->car->type == SEXPR_ATOM, tag, sexpr);
                syntax_assert(cdr->cdr != NULL, tag, sexpr);
                syntax_assert(cdr->cdr->cdr == NULL, tag, sexpr);
                struct env_loc loc = lookup_name(cdr->car->data.atom, env);
                if (loc.frame == -1) {
                    expr->type = EXPR_SET_NAME;
                    expr->data.name_binding.var = cdr->car->data.atom;
                    expr->data.name_binding.val = analyze(cdr->cdr->car, env);
                } else {
                    expr->type = EXPR_SET;
                    expr->data.binding.var = loc;
                    expr->data.binding.val = analyze(cdr->cdr->car, env);
                }
                return expr;
            }

            if (strcmp(tag, "if") == 0) {
                expr->type = EXPR_IF;
                syntax_assert(cdr != NULL, tag, sexpr);
                expr->data.if_data.pred = analyze(cdr->car, env);
                syntax_assert(cdr->cdr != NULL, tag, sexpr);
                expr->data.if_data.conseq = analyze(cdr->cdr->car, env);
                expr->data.if_data.alter =
                    cdr->cdr->cdr ? analyze(cdr->cdr->cdr->car, env) : NULL;
                return expr;
            }

            if (strcmp(tag, "lambda") == 0) {
                expr->type = EXPR_LAMBDA;
                syntax_assert(cdr != NULL, tag, sexpr);
                syntax_assert(cdr->car->type == SEXPR_CONS, tag, sexpr);
                struct name_list *params = analyze_param_list(cdr->car->data.cons, tag, sexpr);
                int len = 0;
                for (struct name_list *params_part = params; params_part != NULL; params_part = params_part->cdr)
                    len++;
                expr->data.lambda.params = len;
                struct name_env *lambda_env = s_malloc(sizeof(struct name_env));
                lambda_env->frame = params;
                lambda_env->next = env;
                expr->data.lambda.body = analyze_list(cdr->cdr, lambda_env);
                return expr;
            }

            if (strcmp(tag, "begin") == 0) {
                expr->type = EXPR_BEGIN;
                expr->data.begin = analyze_list(cdr, env);
                return expr;
            }

            if (strcmp(tag, "quote") == 0) {
                syntax_assert(cdr != NULL, tag, sexpr);
                syntax_assert(cdr->cdr == NULL, tag, sexpr);
                expr->type = EXPR_QUOTE;
                expr->data.quote = cdr->car;
                return expr;
            }

            if (strcmp(tag, "and") == 0) {
                if (cdr == NULL) {
                    expr->type = EXPR_LITERAL;
                    expr->data.literal = (struct val){TYPE_BOOL, {.int_data = 1}};
                    return expr;
                }
                if (cdr->cdr == NULL)
                    return analyze(cdr->car, env);
                struct expr *branch = expr;
                struct sexpr_list *args = cdr;
                while (args != NULL) {
                    branch->type = EXPR_IF;
                    branch->data.if_data.pred = analyze(args->car, env);
                    branch->data.if_data.alter = s_malloc(sizeof(struct expr));
                    branch->data.if_data.alter->type = EXPR_LITERAL;
                    branch->data.if_data.alter->data.literal = (struct val){TYPE_BOOL, {.int_data = 0}};
                    if (args->cdr->cdr == NULL)
                        break;
                    branch->data.if_data.conseq = s_malloc(sizeof(struct expr));
                    branch = branch->data.if_data.conseq;
                    args = args->cdr;
                }
                branch->data.if_data.conseq = analyze(args->cdr->car, env);
                return expr;
            }

            if (strcmp(tag, "or") == 0) {
                return analyze_or_args(cdr, env);
            }

            if (strcmp(tag, "cond") == 0) {
                return analyze_cond_clauses(cdr, sexpr, env);
            }

            if (strcmp(tag, "let") == 0) {
                syntax_assert(cdr != NULL, tag, sexpr);
                syntax_assert(cdr->car->type == SEXPR_CONS, tag, sexpr);
                if (cdr->car->data.cons == NULL)
                    return analyze_seq_expr(cdr->cdr, env);
                struct name_list *params = s_malloc(sizeof(struct name_list));
                struct expr_list *args = s_malloc(sizeof(struct expr_list));
                struct name_list *vars = params;
                struct expr_list *vals = args;
                struct sexpr_list *bindings = cdr->car->data.cons;
                while (bindings != NULL) {
                    syntax_assert(bindings->car->type == SEXPR_CONS, tag, sexpr);
                    syntax_assert(bindings->car->data.cons->cdr != NULL, tag, sexpr);
                    syntax_assert(bindings->car->data.cons->cdr->cdr == NULL, tag, sexpr);
                    syntax_assert(bindings->car->data.cons->car->type == SEXPR_ATOM, tag, sexpr);
                    vars->car = bindings->car->data.cons->car->data.atom;
                    vals->car = analyze(bindings->car->data.cons->cdr->car, env);
                    if (bindings->cdr == NULL)
                        break;
                    vars->cdr = s_malloc(sizeof(struct name_list));
                    vals->cdr = s_malloc(sizeof(struct expr_list));
                    vars = vars->cdr;
                    vals = vals->cdr;
                    bindings = bindings->cdr;
                }
                vars->cdr = NULL;
                vals->cdr = NULL;
                expr->type = EXPR_APPL;
                struct expr *lambda = s_malloc(sizeof(struct expr));
                lambda->type = EXPR_LAMBDA;
                int len = 0;
                for (struct name_list *params_part = params; params_part != NULL; params_part = params_part->cdr)
                    len++;
                lambda->data.lambda.params = len;
                struct name_env *lambda_env = s_malloc(sizeof(struct name_env));
                lambda_env->frame = params;
                lambda_env->next = env;
                lambda->data.lambda.body = analyze_list(cdr->cdr, lambda_env);
                expr->data.appl.proc = lambda;
                expr->data.appl.args = args;
                return expr;
            }

            if (strcmp(tag, "else") == 0) {
                fprintf(stderr, "Syntax error: else is not a special form\n");
                exit(1);
            }

        }

        expr->type = EXPR_APPL;
        expr->data.appl.proc = analyze(sexpr->data.cons->car, env);
        expr->data.appl.args = analyze_list(sexpr->data.cons->cdr, env);
        return expr;
    }
}

struct expr_list *analyze_list(struct sexpr_list *list, struct name_env *env) {
    if (list == NULL)
        return NULL;
    struct expr_list *pair = s_malloc(sizeof(struct expr_list));
    pair->car = analyze(list->car, env);
    pair->cdr = analyze_list(list->cdr, env);
    return pair;
}

/* -- analyze_param_list
 * Analyzes a list of parameters in the form of a `sexpr_list`.
 * Expects all arguments to be atoms.
 * tag and sexpr are arguments used only for reporting errors.
 */
struct name_list *analyze_param_list(struct sexpr_list *list, const char* tag, struct sexpr *sexpr) {
    if (list == NULL)
        return NULL;
    struct name_list *pair = s_malloc(sizeof(struct name_list));
    syntax_assert(list->car->type = SEXPR_ATOM, tag, sexpr);
    pair->car = list->car->data.atom;
    pair->cdr = analyze_param_list(list->cdr, tag, sexpr);
    return pair;
}

/* -- analyze_seq_expr
 * Analyzes a list of `sexpr`s and, unlike `analyze_list` returns an `expr`.
 * Used for transforming cons `sexpr`s into nested if `expr`s.
 */
struct expr *analyze_seq_expr(struct sexpr_list *seq, struct name_env *env) {
    if (seq == NULL) {
        struct expr *expr = s_malloc(sizeof(struct expr));
        expr->type = EXPR_LITERAL;
        expr->data.literal = (struct val){TYPE_VOID};
        return expr;
    }
    if (seq->cdr == NULL)
        return analyze(seq->car, env);
    struct expr *expr = s_malloc(sizeof(struct expr));
    expr->type = EXPR_BEGIN;
    expr->data.begin = analyze_list(seq, env);
    return expr;
}

struct expr *analyze_cond_clauses(struct sexpr_list *clauses, struct sexpr *cond, struct name_env *env) {
    if (clauses == NULL) {
        struct expr *expr = s_malloc(sizeof(struct expr));
        expr->type = EXPR_LITERAL;
        expr->data.literal = (struct val){TYPE_VOID};
        return expr;
    }
    syntax_assert(clauses->car->type == SEXPR_CONS, "cond", cond);

    // else clause
    if (clauses->car->data.cons->car->type == SEXPR_ATOM
            && strcmp(clauses->car->data.cons->car->data.atom, "else") == 0) {
        syntax_assert(clauses->cdr == NULL, "cond", cond);
        return analyze_seq_expr(clauses->car->data.cons->cdr, env);
    }

    // normal cond clause
    struct expr *expr = s_malloc(sizeof(struct expr));
    expr->type = EXPR_IF;
    expr->data.if_data.pred = analyze(clauses->car->data.cons->car, env);
    expr->data.if_data.conseq = analyze_seq_expr(clauses->car->data.cons->cdr, env);
    expr->data.if_data.alter = analyze_cond_clauses(clauses->cdr, cond, env);
    return expr;
}

struct expr *analyze_or_args(struct sexpr_list *args, struct name_env *env) {
//  if (args == NULL) {
//      struct expr *expr = s_malloc(sizeof(struct expr));
//      expr->type = EXPR_LITERAL;
//      expr->data.literal = (struct val){TYPE_BOOL, {.int_data = 0}};
//      return expr;
//  }
//  if (args->cdr == NULL) {
//      return analyze(args->car, env);
//  }
//  struct expr *expr = s_malloc(sizeof(struct expr));
//  expr->type = EXPR_APPL;
//  struct expr *proc = s_malloc(sizeof(struct expr));
//  proc->type = EXPR_LAMBDA;
//  struct name_list *params = s_malloc(sizeof(struct name_list));
//  params->car = "0";
//  params->cdr = NULL;
//  proc->data.lambda.params = params;
//  struct expr_list *body = s_malloc(sizeof(struct expr_list));
//  struct expr *if_test = s_malloc(sizeof(struct expr));
//  if_test->type = EXPR_IF;
//  struct expr *x = s_malloc(sizeof(struct expr));
//  x->type = EXPR_VAR;
//  x->data.var = "0";
//  if_test->data.if_data.pred = x;
//  if_test->data.if_data.conseq = x;
//  if_test->data.if_data.alter = analyze_or_args(args->cdr, env);
//  body->car = if_test;
//  body->cdr = NULL;
//  proc->data.lambda.body = body;
//  expr->data.appl.proc = proc;
//  struct expr_list *appl_args = s_malloc(sizeof(struct expr_list));
//  appl_args->car = analyze(args->car, env);
//  appl_args->cdr = NULL;
//  expr->data.appl.args = appl_args;
//  return expr;
}

/* -- read
 * Performs the entire parsing process.
 * Prevents syntax error on exit.
 */
struct expr* read_expr(void) {
    char c = getchar_nospace();
    if (c == EOF)
        return NULL;
    s_ungetc(c, stdin);
    return analyze(parse(), NULL);
}
