#include <stdio.h>
#include <stdlib.h>

#include "exec.h"
#include "exec_gc.h"
#include "exec_stack.h"
#include "types.h"
#include "env.h"
#include "display.h"
#include "insts.h"
#include "memory.h"
#include "primitives/assert.h"

#ifndef STACK_SIZE
#define STACK_SIZE 65536
#endif

Val stack[STACK_SIZE];
Val *stack_ptr = stack;
Env *exec_env;
Global_env *global_env;

void stack_push(Val val) {
    if (stack_ptr - stack >= STACK_SIZE) {
        eprintf("Error: stack overflow\n");
        exit(1);
    }
    *stack_ptr++ = val;
}

Val stack_pop(void) {
    return *--stack_ptr;
}

/* -- adjust_args
 * For variadic functions, adjusts the number of arguments so that the last
 * argument is a list containing the additional arguments.
 * It also checks if the number of arguments is correct.
 */
static uint32_t adjust_args(uint32_t stack_args, uint32_t req_args) {
    if (req_args & PARAMS_VARIADIC) {
        req_args &= ~PARAMS_VARIADIC;
        args_assert(stack_args >= req_args);
        stack_push((Val){TYPE_NIL});
        for (uint32_t i = 0; i < stack_args - req_args; i++) {
            Pair *pair = gc_alloc(sizeof(Pair));
            pair->cdr = stack_pop();
            pair->car = stack_pop();
            stack_push((Val){TYPE_PAIR, {.pair_data = pair}});
        }
        return req_args + 1;
    } else {
        args_assert(stack_args == req_args);
        return stack_args;
    }
}

/* -- exec
 * Executes the virtual machine instructions, starting at `inst`.
 */
Val exec(uint32_t pc, Global_env *init_global_env) {
    stack_ptr = stack;
    global_env = init_global_env;
    exec_env = NULL;
    while (1) {
        switch (insts[pc].type) {

        case INST_CONST:
            stack_push(insts[pc++].val);
            break;

        case INST_VAR:
            stack_push(locate_var(insts[pc++].var, exec_env, global_env));
            if (stack_ptr[-1].type == TYPE_UNDEF) {
                eprintf("Error: use of undefined value\n");
                exit(1);
            }
            break;

        case INST_NAME: {
            uint32_t index = locate_global_var(insts[pc].name, global_env);
            insts[pc] = (Inst){INST_VAR, {.var = (Env_loc){UINT32_MAX, index}}};
            break;
        }

        case INST_DEF:
            define_var(insts[pc++].name, stack_pop(), global_env);
            stack_push((Val){TYPE_VOID});
            break;

        case INST_SET:
            assign_var(insts[pc++].var, stack_pop(), exec_env, global_env);
            stack_push((Val){TYPE_VOID});
            break;

        case INST_SET_NAME: {
            uint32_t index = locate_global_var(insts[pc].name, global_env);
            insts[pc] = (Inst){INST_SET, {.var = (Env_loc){UINT32_MAX, index}}};
            break;
        }

        case INST_JUMP:
            pc = insts[pc].index;
            break;

        case INST_JUMP_FALSE: {
            Val v = stack_pop();
            if (v.type == TYPE_BOOL && v.int_data == 0)
                pc = insts[pc].index;
            else
                pc++;
            break;
        }

        case INST_LAMBDA: {
            Lambda *lambda = gc_alloc(sizeof(Lambda));
            lambda->params = insts[pc].lambda.params;
            lambda->body = insts[pc].lambda.index;
            lambda->env = exec_env;
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
                uint32_t vals_num = adjust_args(insts[pc].num, op->lambda_data->params);
                uint32_t new_pc = op->lambda_data->body;
                Env *lambda_env = extend_env(op + 1, vals_num, op->lambda_data->env);
                stack_ptr = op;
                stack_push((Val){TYPE_ENV, {.env_data = exec_env}});
                stack_push((Val){TYPE_INST, {.inst_data = pc + 1}});
                exec_env = lambda_env;
                pc = new_pc;
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
                High_prim *high_prim = op->high_prim_data;
                *(op + 2) = *op;
                *op = (Val){TYPE_ENV, {.env_data = exec_env}};
                *(op + 1) = (Val){TYPE_INST, {.inst_data = pc + 1}};
                High_prim_return r = high_prim(op + 3, insts[pc].num);
                if (r.global_env != NULL && r.global_env != global_env) {
                    stack_push((Val){TYPE_GLOBAL_ENV, {.global_env_data = global_env}});
                    global_env = r.global_env;
                }
                pc = r.pc;
                break;
            }
            default:
                eprintf("Error: %s is not an applicable type\n", type_name(op->type));
                exit(1);
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
                uint32_t vals_num = adjust_args(insts[pc].num, op->lambda_data->params);
                exec_env = extend_env(op + 1, vals_num, op->lambda_data->env);
                stack_ptr = op;
                pc = op->lambda_data->body;
                break;
            }
            case TYPE_HIGH_PRIM: {
                High_prim_return r = op->high_prim_data(op + 1, insts[pc].num);
                if (r.global_env != NULL && r.global_env != global_env) {
                    stack_push((Val){TYPE_GLOBAL_ENV, {.global_env_data = global_env}});
                    global_env = r.global_env;
                }
                pc = r.pc;
                break;
            }
            default:
                eprintf("Error: %s is not an applicable type\n", type_name(op->type));
                exit(1);
            }
            break;
        }

        case INST_RETURN: {
            Val result = stack_pop();
            if (stack_ptr == stack)
                return result;
            Val v = stack_pop();
            if (v.type == TYPE_GLOBAL_ENV) {
                global_env = v.global_env_data;
                if (stack_ptr == stack)
                    return result;
                v = stack_pop();
            }
            pc = v.inst_data;
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
            eprintf("Error: unrecognized instruction type %d\n", insts[pc].type);
            exit(1);

        }
    }
}
