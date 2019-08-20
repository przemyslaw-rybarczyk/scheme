#include <stddef.h>

#include "compiler.h"
#include "assert.h"
#include "../expr.h"
#include "../insts.h"
#include "../parser.h"
#include "../symbol.h"

FILE *compiler_input_file;

Val next_token_prim(Val *args, int num) {
    args_assert(num == 0);
    return get_token(compiler_input_file);
}

Val this_inst_prim(Val *args, int num) {
    return (Val){TYPE_INT, {.int_data = this_inst()}};
}

Val next_inst_prim(Val *args, int num) {
    return (Val){TYPE_INT, {.int_data = next_inst()}};
}

Val set_const_prim(Val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[args[0].int_data] = (Inst){INST_CONST, {.val = args[1]}};
    return (Val){TYPE_VOID};
}

Val set_var_prim(Val *args, int num) {
    args_assert(num == 3);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    if (args[2].type != TYPE_INT)
        type_error(args[2]);
    insts[args[0].int_data] = (Inst){INST_VAR, {.var = (Env_loc){args[1].int_data, args[2].int_data}}};
    return (Val){TYPE_VOID};
}

Val set_name_prim(Val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_NAME, {.name = args[1].string_data}};
    return (Val){TYPE_VOID};
}

Val set_def_prim(Val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_DEF, {.name = args[1].string_data}};
    return (Val){TYPE_VOID};
}

Val set_set_prim(Val *args, int num) {
    args_assert(num == 3);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    if (args[2].type != TYPE_INT)
        type_error(args[2]);
    insts[args[0].int_data] = (Inst){INST_SET, {.var = (Env_loc){args[1].int_data, args[2].int_data}}};
    return (Val){TYPE_VOID};
}

Val set_set_name_prim(Val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_SET_NAME, {.name = args[1].string_data}};
    return (Val){TYPE_VOID};
}

Val set_jump_prim(Val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_JUMP, {.num = args[1].int_data}};
    return (Val){TYPE_VOID};
}

Val set_jump_false_prim(Val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_JUMP_FALSE, {.num = args[1].int_data}};
    return (Val){TYPE_VOID};
}

Val set_lambda_prim(Val *args, int num) {
    args_assert(num == 3);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    if (args[2].type != TYPE_INT)
        type_error(args[2]);
    insts[args[0].int_data] = (Inst){INST_LAMBDA, {.lambda = {args[1].int_data, args[2].int_data}}};
    return (Val){TYPE_VOID};
}

Val set_call_prim(Val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_CALL, {.num = args[1].int_data}};
    return (Val){TYPE_VOID};
}

Val set_tail_call_prim(Val *args, int num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_TAIL_CALL, {.num = args[1].int_data}};
    return (Val){TYPE_VOID};
}

Val set_return_prim(Val *args, int num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[args[0].int_data] = (Inst){INST_RETURN};
    return (Val){TYPE_VOID};
}

Val set_delete_prim(Val *args, int num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[args[0].int_data] = (Inst){INST_DELETE};
    return (Val){TYPE_VOID};
}

Val set_cons_prim(Val *args, int num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[args[0].int_data] = (Inst){INST_CONS};
    return (Val){TYPE_VOID};
}

// awful workaround for or until macros are implemented
Val zero_symbol_prim(Val *args, int num) {
    args_assert(num == 0);
    return (Val){TYPE_SYMBOL, {.string_data = intern_symbol("0")}};
}

struct prim_binding compiler_prims[] = {
    "next-token", next_token_prim,
    "this-inst", this_inst_prim,
    "next-inst", next_inst_prim,
    "set-const!", set_const_prim,
    "set-var!", set_var_prim,
    "set-name!", set_name_prim,
    "set-def!", set_def_prim,
    "set-set!", set_set_prim,
    "set-set-name!", set_set_name_prim,
    "set-jump!", set_jump_prim,
    "set-jump-false!", set_jump_false_prim,
    "set-lambda!", set_lambda_prim,
    "set-call!", set_call_prim,
    "set-tail-call!", set_tail_call_prim,
    "set-return!", set_return_prim,
    "set-delete!", set_delete_prim,
    "set-cons!", set_cons_prim,
    "zero-symbol", zero_symbol_prim,
};

size_t compiler_prims_size = sizeof(compiler_prims) / sizeof(struct prim_binding);
