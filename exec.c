#include <stdio.h>
#include <stdlib.h>

#include "exec.h"
#include "expr.h"
#include "env.h"
#include "memory.h"
#include "display.h"

#define STACK_SIZE 65536

struct val stack[STACK_SIZE];
struct val *stack_ptr;
struct env *exec_env;

struct inst return_inst = (struct inst){INST_RETURN};

void stack_push(struct val val) {
    if (stack_ptr - stack >= STACK_SIZE) {
        fprintf(stderr, "Error: stack overflow\n");
        exit(2);
    }
    *stack_ptr++ = val;
}

struct val stack_pop(void) {
    return *--stack_ptr;
}

/* -- exec
 * Executes the virual machine instructions, starting at `inst`.
 */
struct val exec(struct inst *inst) {
    stack_ptr = stack;
    exec_env = NULL;
    while (1) {
        switch (inst->type) {

        case INST_CONST:
            stack_push(inst++->args.val);
            break;

        case INST_VAR:
            stack_push(lookup_var(inst++->args.name, exec_env));
            break;

        case INST_DEF:
            define_var(inst++->args.name, stack_ptr - 1, exec_env);
            stack_pop();
            stack_push((struct val){TYPE_VOID});
            break;

        case INST_SET:
            assign_var(inst++->args.name, stack_pop(), exec_env);
            stack_push((struct val){TYPE_VOID});
            break;

        case INST_JUMP:
            inst = inst->args.ptr;
            break;

        case INST_JUMP_FALSE:
            if (!is_true(stack_pop()))
                inst = inst->args.ptr;
            else
                inst++;
            break;

        case INST_LAMBDA: {
            struct lambda *lambda = alloc_lambda();
            lambda->params = inst->args.lambda.params;
            lambda->body = inst->args.lambda.ptr;
            lambda->env = exec_env;
            lambda->new_ptr = NULL;
            stack_push((struct val){TYPE_LAMBDA, {.lambda_data = lambda}});
            inst++;
            break;
        }

        case INST_CALL: {
            struct val *op = stack_ptr - inst->args.num - 1;
            switch (op->type) {
            case TYPE_PRIM: {
                struct val result = op->data.prim_data(op + 1, inst->args.num);
                stack_ptr = op;
                stack_push(result);
                inst++;
                break;
            }
            case TYPE_LAMBDA: {
                struct lambda *lambda = op->data.lambda_data;
                struct inst *old_inst = inst;
                inst = lambda->body;
                struct env *lambda_env = extend_env(lambda->params, op + 1, old_inst->args.num, lambda->env);
                stack_ptr = op;
                stack_push((struct val){TYPE_ENV, {.env_data = exec_env}});
                stack_push((struct val){TYPE_INST, {.inst_data = old_inst + 1}});
                exec_env = lambda_env;
                break;
            }
            case TYPE_HIGH_PRIM: {
                stack_push((struct val){}); // ensure error in case of overflow
                stack_push((struct val){});
                stack_pop();
                stack_pop();
                for (struct val *arg_ptr = stack_ptr - 1; arg_ptr > op; arg_ptr--)
                    *(arg_ptr + 2) = *arg_ptr;
                stack_ptr += 2;
                struct inst *(*high_prim)(int) = op->data.high_prim_data;
                *(op + 2) = *op;
                *op = (struct val){TYPE_ENV, {.env_data = exec_env}};
                *(op + 1) = (struct val){TYPE_INST, {.inst_data = inst + 1}};
                inst = high_prim(inst->args.num);
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
            struct val *op = stack_ptr - inst->args.num - 1;
            switch (op->type) {
            case TYPE_PRIM: {
                struct val result = op->data.prim_data(op + 1, inst->args.num);
                stack_ptr = op;
                stack_push(result);
                inst = &return_inst;
                break;
            }
            case TYPE_LAMBDA: {
                struct lambda *lambda = op->data.lambda_data;
                exec_env = extend_env(lambda->params, op + 1, inst->args.num, lambda->env);
                stack_ptr = op;
                inst = op->data.lambda_data->body;
                break;
            }
            case TYPE_HIGH_PRIM:
                inst = op->data.high_prim_data(inst->args.num);
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
            struct val result = stack_pop();
            inst = stack_pop().data.inst_data;
            exec_env = stack_pop().data.env_data;
            stack_push(result);
            break;
        }

        case INST_DELETE:
            stack_pop();
            inst++;
            break;

        case INST_CONS: {
            struct pair *pair = alloc_pair();
            pair->cdr = stack_pop();
            pair->car = stack_pop();
            stack_push((struct val){TYPE_PAIR, {.pair_data = pair}});
            inst++;
            break;
        }

        }
    }
}

int is_true(struct val val) {
    return !(val.type == TYPE_BOOL && val.data.int_data == 0);
}
