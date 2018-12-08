#include <stdio.h>
#include <stdlib.h>

#include "exec.h"
#include "expr.h"
#include "env.h"
#include "memory.h"
#include "display.h"
#include "insts.h"

#define STACK_SIZE 65536

Val stack[STACK_SIZE];
Val *stack_ptr;
Env *exec_env;
int pc;

void stack_push(Val val) {
    if (stack_ptr - stack >= STACK_SIZE) {
        fprintf(stderr, "Error: stack overflow\n");
        exit(2);
    }
    *stack_ptr++ = val;
}

Val stack_pop(void) {
    return *--stack_ptr;
}

/* -- exec
 * Executes the virual machine instructions, starting at `inst`.
 */
Val exec(long init_pc) {
    stack_ptr = stack;
    exec_env = NULL;
    pc = init_pc;
    while (1) {
        switch (insts[pc].type) {

        case INST_CONST:
            stack_push(insts[pc++].val);
            break;

        case INST_VAR:
            stack_push(locate_var(insts[pc++].var, exec_env));
            break;

        case INST_NAME: {
            int index = locate_global_var(insts[pc].name);
            insts[pc] = (Inst){INST_VAR, {.var = (Env_loc){-1, index}}};
            break;
        }

        case INST_DEF:
            define_var(insts[pc++].name, stack_pop(), exec_env);
            stack_push((Val){TYPE_VOID});
            break;

        case INST_SET:
            assign_var(insts[pc++].var, stack_pop(), exec_env);
            stack_push((Val){TYPE_VOID});
            break;

        case INST_SET_NAME: {
            int index = locate_global_var(insts[pc].name);
            insts[pc] = (Inst){INST_SET, {.var = (Env_loc){-1, index}}};
            break;
        }

        case INST_JUMP:
            pc = insts[pc].index;
            break;

        case INST_JUMP_FALSE:
            if (!is_true(stack_pop()))
                pc = insts[pc].index;
            else
                pc++;
            break;

        case INST_LAMBDA: {
            Lambda *lambda = gc_alloc(sizeof(Lambda));
            lambda->params = insts[pc].lambda.params;
            lambda->body = insts[pc].lambda.index;
            lambda->env = exec_env;
            lambda->new_ptr = NULL;
            stack_push((Val){TYPE_LAMBDA, {.lambda_data = lambda}});
            pc++;
            break;
        }

        case INST_CALL: {
            Val *op = stack_ptr - insts[pc].num - 1;
            switch (op->type) {
            case TYPE_PRIM: {
                Val result = op->prim_data(op + 1, insts[pc].num);
                stack_ptr = op;
                stack_push(result);
                pc++;
                break;
            }
            case TYPE_LAMBDA: {
                Lambda *lambda = op->lambda_data;
                int old_pc = pc;
                pc = lambda->body;
                Env *lambda_env = extend_env(op + 1, insts[old_pc].num, lambda->env);
                stack_ptr = op;
                stack_push((Val){TYPE_ENV, {.env_data = exec_env}});
                stack_push((Val){TYPE_INST, {.inst_data = old_pc + 1}});
                exec_env = lambda_env;
                break;
            }
            case TYPE_HIGH_PRIM: {
                stack_push((Val){}); // ensure error in case of overflow
                stack_push((Val){});
                stack_pop();
                stack_pop();
                for (Val *arg_ptr = stack_ptr - 1; arg_ptr > op; arg_ptr--)
                    *(arg_ptr + 2) = *arg_ptr;
                stack_ptr += 2;
                int (*high_prim)(int) = op->high_prim_data;
                *(op + 2) = *op;
                *op = (Val){TYPE_ENV, {.env_data = exec_env}};
                *(op + 1) = (Val){TYPE_INST, {.inst_data = pc + 1}};
                pc = high_prim(insts[pc].num);
                break;
            }
            default:
                fprintf(stderr, "Error: %s is not an applicable type\n",
                        sprint_type(op->type));
                exit(2);
            }
            break;
        }

        case INST_TAIL_CALL: {
            Val *op = stack_ptr - insts[pc].num - 1;
            switch (op->type) {
            case TYPE_PRIM: {
                Val result = op->prim_data(op + 1, insts[pc].num);
                stack_ptr = op;
                stack_push(result);
                pc = return_inst;
                break;
            }
            case TYPE_LAMBDA: {
                Lambda *lambda = op->lambda_data;
                exec_env = extend_env(op + 1, insts[pc].num, lambda->env);
                stack_ptr = op;
                pc = op->lambda_data->body;
                break;
            }
            case TYPE_HIGH_PRIM:
                pc = op->high_prim_data(insts[pc].num);
                break;
            default:
                fprintf(stderr, "Error: %s is not an applicable type\n",
                        sprint_type(op->type));
                exit(2);
            }
            break;
        }

        case INST_RETURN: {
            if (stack_ptr == stack + 1)
                return stack_pop();
            Val result = stack_pop();
            pc = stack_pop().inst_data;
            exec_env = stack_pop().env_data;
            stack_push(result);
            break;
        }

        case INST_DELETE:
            stack_pop();
            pc++;
            break;

        case INST_CONS: {
            Pair *pair = gc_alloc(sizeof(Pair));
            pair->cdr = stack_pop();
            pair->car = stack_pop();
            stack_push((Val){TYPE_PAIR, {.pair_data = pair}});
            pc++;
            break;
        }

        case INST_EXPR:
            pc++;
            break;

        default:
            fprintf(stderr, "Error\n");
            // TODO
            exit(-1);

        }
    }
}

int is_true(Val val) {
    return !(val.type == TYPE_BOOL && val.int_data == 0);
}
