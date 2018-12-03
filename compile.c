#include <stddef.h>

#include "compile.h"
#include "expr.h"
#include "insts.h"
#include "symbol.h"

void compile_list(struct expr_list *exprs, int tail);

/* -- compile
 * Compiles an expr and puts the resulting VM code in program memory (insts.c).
 */
void compile(struct expr *expr, int tail) {
    switch (expr->type) {

    case EXPR_LITERAL:
        *next_inst() = (struct inst){INST_CONST, {.val = expr->data.literal}};
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        break;

    case EXPR_VAR:
        *next_inst() = (struct inst){INST_VAR, {.var = expr->data.var}};
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        break;

    case EXPR_NAME:
        *next_inst() = (struct inst){INST_NAME, {.name = expr->data.name}};
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        break;

    case EXPR_DEF:
        compile(expr->data.name_binding.val, 0);
        *next_inst() = (struct inst){INST_DEF, {.name = expr->data.name_binding.var}};
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        break;

    case EXPR_SET:
        compile(expr->data.binding.val, 0);
        *next_inst() = (struct inst){INST_SET, {.var = expr->data.binding.var}};
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        break;

    case EXPR_SET_NAME:
        compile(expr->data.name_binding.val, 0);
        *next_inst() = (struct inst){INST_SET_NAME, {.name = expr->data.name_binding.var}};
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        break;

    case EXPR_IF:
        compile(expr->data.if_data.pred, 0);
        struct inst *jump_false = next_inst();
        jump_false->type = INST_JUMP_FALSE;
        compile(expr->data.if_data.conseq, tail);
        struct inst *jump_true = next_inst();
        jump_true->type = INST_JUMP;
        jump_false->args.ptr = this_inst();
        if (expr->data.if_data.alter != NULL)
            compile(expr->data.if_data.alter, tail);
        else {
            *next_inst() = (struct inst){INST_CONST, {.val = (struct val){TYPE_VOID}}};
            if (tail)
                *next_inst() = (struct inst){INST_RETURN};
        }
        jump_true->args.ptr = this_inst();
        break;

    case EXPR_LAMBDA: {
        struct inst *jump_after = next_inst();
        jump_after->type = INST_JUMP;
        struct inst *lambda_address = this_inst();
        compile_list(expr->data.lambda.body, 1);
        struct inst *lambda_inst = jump_after->args.ptr = next_inst();
        lambda_inst->type = INST_LAMBDA;
        lambda_inst->args.lambda.params = expr->data.lambda.params;
        lambda_inst->args.lambda.ptr = lambda_address;
        jump_after->args.ptr = lambda_inst;
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        break;
    }

    case EXPR_BEGIN:
        compile_list(expr->data.begin, tail);
        break;

    case EXPR_APPL: {
        compile(expr->data.appl.proc, 0);
        int num = 0;
        for (struct expr_list *args = expr->data.appl.args; args != NULL; args = args->cdr) {
            compile(args->car, 0);
            num++;
        }
        *next_inst() = (struct inst){tail ? INST_TAIL_CALL : INST_CALL, {.num = num}};
        break;
    }

    case EXPR_QUOTE:
        compile_quote(expr->data.quote);
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        break;

    }
}

void compile_list(struct expr_list *exprs, int tail) {
    if (exprs == NULL) {
        if (tail)
            *next_inst() = (struct inst){INST_RETURN};
        return;
    }
    for (; exprs->cdr != NULL; exprs = exprs->cdr) {
        compile(exprs->car, 0);
        *next_inst() = (struct inst){INST_DELETE};
    }
    compile(exprs->car, tail);
}

void compile_quote_list(struct sexpr_list *sexprs);

void compile_quote(struct sexpr *sexpr) {
    switch (sexpr->type) {
    case SEXPR_LITERAL:
        *next_inst() = (struct inst){INST_CONST, {.val = sexpr->data.literal}};
        break;
    case SEXPR_ATOM:
        *next_inst() = (struct inst){INST_CONST, {.val = (struct val){TYPE_SYMBOL, {.string_data = intern_symbol(sexpr->data.atom)}}}};
        break;
    case SEXPR_CONS:
        compile_quote_list(sexpr->data.cons);
        break;
    }
}

void compile_quote_list(struct sexpr_list *sexprs) {
    if (sexprs == NULL) {
        *next_inst() = (struct inst){INST_CONST, {.val = (struct val){TYPE_NIL}}};
        return;
    }
    compile_quote(sexprs->car);
    compile_quote_list(sexprs->cdr);
    *next_inst() = (struct inst){INST_CONS};
}
