#include <stdio.h>
#include <stdlib.h>

#include "eval.h"
#include "expr.h"
#include "env.h"
#include "safemem.h"
#include "display.h"
#include "memory.h"
#include "symbol.h"

struct val eval_sequence(struct expr_list *seq, struct env *env);
struct val_list *eval_arg_list(struct expr_list *list, struct env *env);
struct val eval_and_exprs(struct expr_list *exprs, struct env *env);
struct val eval_or_exprs(struct expr_list *exprs, struct env *env);
void free_arg_list(struct val_list *list);

/* -- eval
 * Evaluates an expression and returns its value.
 */
struct val eval(struct expr *expr, struct env *env) {
    switch (expr->type) {

    case EXPR_LITERAL:
        return expr->data.literal;

    case EXPR_VAR:
        return lookup_var(expr->data.var, env);

    case EXPR_DEF: {
        struct env **gc_env = gc_push_env(env);
        struct val val = eval(expr->data.binding.val, env);
        gc_pop_env();
        return define_var(expr->data.binding.var, val, *gc_env);
    }

    case EXPR_SET: {
        struct env **gc_env = gc_push_env(env);
        struct val val = eval(expr->data.binding.val, env);
        gc_pop_env();
        return assign_var(expr->data.binding.var, val, *gc_env);
    }

    case EXPR_IF: {
        struct env **gc_env = gc_push_env(env);
        int pred = is_true(eval(expr->data.if_data.pred, env));
        gc_pop_env();
        if (pred)
            return eval(expr->data.if_data.conseq, *gc_env);
        else {
            struct expr *alter = expr->data.if_data.alter;
            if (alter != NULL)
                return eval(alter, *gc_env);
            else
                return (struct val){TYPE_VOID};
        }
    }

    case EXPR_LAMBDA: {
        struct env **gc_env = gc_push_env(env);
        struct lambda *lambda = alloc_lambda();
        lambda->params = expr->data.lambda.params;
        lambda->body = expr->data.lambda.body;
        lambda->env = *gc_env;
        lambda->new_ptr = NULL;
        gc_pop_env();
        return (struct val){TYPE_LAMBDA, {.lambda_data = lambda}};
    }

    case EXPR_BEGIN:
        return eval_sequence(expr->data.begin, env);

    case EXPR_QUOTE:
        return eval_quote(expr->data.quote);

    case EXPR_AND:
        return eval_and_exprs(expr->data.begin, env);

    case EXPR_OR:
        return eval_or_exprs(expr->data.begin, env);

    case EXPR_APPL: {
        struct env **gc_env = gc_push_env(env);
        struct val_list *args = eval_arg_list(expr->data.appl.args, *gc_env);
        gc_pop_env();
        struct val proc = eval(expr->data.appl.proc, *gc_env);
        return apply(proc, args);
    }

    }
}

/* -- apply
 * Applies a procedure to a list of arguments and returns the resulting value.
 * The high-order primitives are treated differently here - as they may call
 * other functions tail-recursively, it is their responsibility to free the
 * argument list before the call.
 */
struct val apply(struct val proc, struct val_list *args) {
    switch (proc.type) {
    case TYPE_PRIM: {
        struct val result = proc.data.prim_data(args);
        free_arg_list(args);
        return result;
    }
    case TYPE_LAMBDA: {
        struct expr_list *proc_body = proc.data.lambda_data->body;
        struct env *ext_env =
            extend_env(proc.data.lambda_data->params, args,
                    proc.data.lambda_data->env);
        free_arg_list(args);
        return eval_sequence(proc_body, ext_env);
    }
    case TYPE_HIGH_PRIM:
        return proc.data.prim_data(args);
    default:
        fprintf(stderr, "Error: %s is not an applicable type\n",
                sprint_type(proc.type));
        exit(2);
    }
}

/* -- eval_sequence
 * Evaluates a list of expressions, returning the value of the last one.
 * Returns void if there are no expressions.
 */
struct val eval_sequence(struct expr_list* seq, struct env *env) {
    if (seq == NULL)
        return (struct val){TYPE_VOID};
    struct env **gc_env = gc_push_env(env);
    while (seq->cdr != NULL) {
        eval(seq->car, *gc_env);
        seq = seq->cdr;
    }
    gc_pop_env();
    return eval(seq->car, *gc_env);
}

/* -- eval_arg_list
 * Evaluates a list of expressions and combines them into a list.
 * Pushes all the resulting values onto the GC val stack; they should
 * be freed by calling `free_arg_list`.
 * Used for evaluating arguments to a function.
 */
struct val_list *eval_arg_list(struct expr_list *list, struct env *env) {
    if (list == NULL)
        return NULL;
    struct val_list *result = s_malloc(sizeof(struct val_list));
    struct val_list *pair = result;
    struct env **gc_env = gc_push_env(env);
    while (list->cdr != NULL) {
        pair->car = eval(list->car, *gc_env);
        gc_push_val(&pair->car);
        pair->cdr = s_malloc(sizeof(struct val_list));
        pair = pair->cdr;
        list = list->cdr;
    }
    gc_pop_env();
    pair->car = eval(list->car, *gc_env);
    gc_push_val(&pair->car);
    pair->cdr = NULL;
    return result;
}

/* -- eval_and_exprs
 * Evaluates expressions used as values to an and expression.
 * Returns false on the first expression evaluating to false,
 * otherwise the value of the last expression or true if there
 * are no expressions.
 */
struct val eval_and_exprs(struct expr_list *exprs, struct env *env) {
    if (exprs == NULL)
        return (struct val){TYPE_BOOL, {.int_data = 1}};
    if (exprs->cdr == NULL)
        return eval(exprs->car, env);
    struct env **gc_env = gc_push_env(env);
    if (is_true(eval(exprs->car, env)))
        return eval_and_exprs(exprs->cdr, *gc_env);
    return (struct val){TYPE_BOOL, {.int_data = 0}};
}

/* -- eval_or_exprs
 * Evaluates expressions used as values to an or expression.
 * Returns the value of the first true expression, or false if all are false
 * or if there are no expressions.
 */
struct val eval_or_exprs(struct expr_list *exprs, struct env *env) {
    if (exprs == NULL)
        return (struct val){TYPE_BOOL, {.int_data = 0}};
    struct env **gc_env = gc_push_env(env);
    struct val val = eval(exprs->car, env);
    if (is_true(val))
        return val;
    return eval_or_exprs(exprs->cdr, *gc_env);
}

/* -- eval_quote
 * Converts the argument to a quote special form into a value.
 */
struct val quote_list(struct sexpr_list *list);

struct val eval_quote(struct sexpr *sexpr) {
    switch (sexpr->type) {
    case SEXPR_LITERAL:
        return sexpr->data.literal;
    case SEXPR_ATOM:
        return (struct val){TYPE_SYMBOL, {.string_data = intern_symbol(sexpr->data.atom)}};
    case SEXPR_CONS:
        return quote_list(sexpr->data.cons);
    }
}

struct val quote_list(struct sexpr_list *list) {
    if (list == NULL)
        return (struct val){TYPE_NIL};
    struct val val = (struct val){TYPE_PAIR, {.pair_data = alloc_pair()}};
    val.data.pair_data->car = (struct val){TYPE_NIL};
    val.data.pair_data->cdr = (struct val){TYPE_NIL};
    gc_push_val(&val);
    struct val car = eval_quote(list->car);
    val.data.pair_data->car = car;
    struct val cdr = quote_list(list->cdr);
    val.data.pair_data->cdr = cdr;
    gc_pop_val();
    return val;
}


/* -- free_arg_list
 * Frees a val_list.
 * Should be used for lists allocated with `eval_arg_list`.
 */
void free_arg_list(struct val_list *list) {
    if (list == NULL)
        return;
    gc_pop_val();
    free_arg_list(list->cdr);
    free(list);
}

/* -- is_true
 * Returns true if the object is not equal to false.
 */
int is_true(struct val val) {
    return !(val.type == TYPE_BOOL && val.data.int_data == 0);
}
