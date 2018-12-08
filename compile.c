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
        insts[next_inst()] = (Inst){INST_CONST, {.val = expr->literal}};
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        break;

    case EXPR_VAR:
        insts[next_inst()] = (Inst){INST_VAR, {.var = expr->var}};
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        break;

    case EXPR_NAME:
        insts[next_inst()] = (Inst){INST_NAME, {.name = expr->name}};
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        break;

    case EXPR_DEF:
        compile(expr->name_binding.val, 0);
        insts[next_inst()] = (Inst){INST_DEF, {.name = expr->name_binding.var}};
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        break;

    case EXPR_SET:
        compile(expr->binding.val, 0);
        insts[next_inst()] = (Inst){INST_SET, {.var = expr->binding.var}};
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        break;

    case EXPR_SET_NAME:
        compile(expr->name_binding.val, 0);
        insts[next_inst()] = (Inst){INST_SET_NAME, {.name = expr->name_binding.var}};
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        break;

    case EXPR_IF:
        compile(expr->if_data.pred, 0);
        int jump_false = next_inst();
        compile(expr->if_data.conseq, tail);
        int jump_true = next_inst();
        insts[jump_false] = (Inst){INST_JUMP_FALSE, {.index = this_inst()}};
        if (expr->if_data.alter != NULL)
            compile(expr->if_data.alter, tail);
        else {
            insts[next_inst()] = (Inst){INST_CONST, {.val = (Val){TYPE_VOID}}};
            if (tail)
                insts[next_inst()] = (Inst){INST_RETURN};
        }
        insts[jump_true] = (Inst){INST_JUMP, {.index = this_inst()}};
        break;

    case EXPR_LAMBDA: {
        int jump_after = next_inst();
        int lambda_address = this_inst();
        compile_list(expr->lambda.body, 1);
        int lambda_inst = next_inst();
        insts[lambda_inst] = (Inst){INST_LAMBDA, {.lambda = {expr->lambda.params, lambda_address}}};
        insts[jump_after] = (Inst){INST_JUMP, {.index = lambda_inst}};
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        break;
    }

    case EXPR_BEGIN:
        compile_list(expr->begin, tail);
        break;

    case EXPR_APPL: {
        compile(expr->appl.proc, 0);
        int num = 0;
        for (struct expr_list *args = expr->appl.args; args != NULL; args = args->cdr) {
            compile(args->car, 0);
            num++;
        }
        insts[next_inst()] = (Inst){tail ? INST_TAIL_CALL : INST_CALL, {.num = num}};
        break;
    }

    case EXPR_QUOTE:
        compile_quote(expr->quote);
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        break;

    }
}

void compile_list(struct expr_list *exprs, int tail) {
    if (exprs == NULL) {
        if (tail)
            insts[next_inst()] = (Inst){INST_RETURN};
        return;
    }
    for (; exprs->cdr != NULL; exprs = exprs->cdr) {
        compile(exprs->car, 0);
        insts[next_inst()] = (Inst){INST_DELETE};
    }
    compile(exprs->car, tail);
}

void compile_quote_list(struct sexpr_list *sexprs);

void compile_quote(struct sexpr *sexpr) {
    switch (sexpr->type) {
    case SEXPR_LITERAL:
        insts[next_inst()] = (Inst){INST_CONST, {.val = sexpr->literal}};
        break;
    case SEXPR_ATOM:
        insts[next_inst()] = (Inst){INST_CONST, {.val = (Val){TYPE_SYMBOL, {.string_data = intern_symbol(sexpr->atom)}}}};
        break;
    case SEXPR_CONS:
        compile_quote_list(sexpr->cons);
        break;
    }
}

void compile_quote_list(struct sexpr_list *sexprs) {
    if (sexprs == NULL) {
        insts[next_inst()] = (Inst){INST_CONST, {.val = (Val){TYPE_NIL}}};
        return;
    }
    compile_quote(sexprs->car);
    compile_quote_list(sexprs->cdr);
    insts[next_inst()] = (Inst){INST_CONS};
}
