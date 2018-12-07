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
        insts[next_inst()] = (struct inst){INST_CONST, {.val = expr->data.literal}};
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
        break;

    case EXPR_VAR:
        insts[next_inst()] = (struct inst){INST_VAR, {.var = expr->data.var}};
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
        break;

    case EXPR_NAME:
        insts[next_inst()] = (struct inst){INST_NAME, {.name = expr->data.name}};
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
        break;

    case EXPR_DEF:
        compile(expr->data.name_binding.val, 0);
        insts[next_inst()] = (struct inst){INST_DEF, {.name = expr->data.name_binding.var}};
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
        break;

    case EXPR_SET:
        compile(expr->data.binding.val, 0);
        insts[next_inst()] = (struct inst){INST_SET, {.var = expr->data.binding.var}};
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
        break;

    case EXPR_SET_NAME:
        compile(expr->data.name_binding.val, 0);
        insts[next_inst()] = (struct inst){INST_SET_NAME, {.name = expr->data.name_binding.var}};
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
        break;

    case EXPR_IF:
        compile(expr->data.if_data.pred, 0);
        int jump_false = next_inst();
        compile(expr->data.if_data.conseq, tail);
        int jump_true = next_inst();
        insts[jump_false] = (struct inst){INST_JUMP_FALSE, {.index = this_inst()}};
        if (expr->data.if_data.alter != NULL)
            compile(expr->data.if_data.alter, tail);
        else {
            insts[next_inst()] = (struct inst){INST_CONST, {.val = (struct val){TYPE_VOID}}};
            if (tail)
                insts[next_inst()] = (struct inst){INST_RETURN};
        }
        insts[jump_true] = (struct inst){INST_JUMP, {.index = this_inst()}};
        break;

    case EXPR_LAMBDA: {
        int jump_after = next_inst();
        int lambda_address = this_inst();
        compile_list(expr->data.lambda.body, 1);
        int lambda_inst = next_inst();
        insts[lambda_inst] = (struct inst){INST_LAMBDA, {.lambda = {expr->data.lambda.params, lambda_address}}};
        insts[jump_after] = (struct inst){INST_JUMP, {.index = lambda_inst}};
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
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
        insts[next_inst()] = (struct inst){tail ? INST_TAIL_CALL : INST_CALL, {.num = num}};
        break;
    }

    case EXPR_QUOTE:
        compile_quote(expr->data.quote);
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
        break;

    }
}

void compile_list(struct expr_list *exprs, int tail) {
    if (exprs == NULL) {
        if (tail)
            insts[next_inst()] = (struct inst){INST_RETURN};
        return;
    }
    for (; exprs->cdr != NULL; exprs = exprs->cdr) {
        compile(exprs->car, 0);
        insts[next_inst()] = (struct inst){INST_DELETE};
    }
    compile(exprs->car, tail);
}

void compile_quote_list(struct sexpr_list *sexprs);

void compile_quote(struct sexpr *sexpr) {
    switch (sexpr->type) {
    case SEXPR_LITERAL:
        insts[next_inst()] = (struct inst){INST_CONST, {.val = sexpr->data.literal}};
        break;
    case SEXPR_ATOM:
        insts[next_inst()] = (struct inst){INST_CONST, {.val = (struct val){TYPE_SYMBOL, {.string_data = intern_symbol(sexpr->data.atom)}}}};
        break;
    case SEXPR_CONS:
        compile_quote_list(sexpr->data.cons);
        break;
    }
}

void compile_quote_list(struct sexpr_list *sexprs) {
    if (sexprs == NULL) {
        insts[next_inst()] = (struct inst){INST_CONST, {.val = (struct val){TYPE_NIL}}};
        return;
    }
    compile_quote(sexprs->car);
    compile_quote_list(sexprs->cdr);
    insts[next_inst()] = (struct inst){INST_CONS};
}
