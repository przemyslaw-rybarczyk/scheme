#include <stddef.h>
#include <stdio.h>

#include "compiler.h"
#include "../types.h"
#include "../insts.h"
#include "../parser.h"
#include "../safestd.h"
#include "../symbol.h"
#include "assert.h"

FILE *compiler_input_file;

Val next_token_prim(Val *args, uint32_t num) {
    args_assert(num == 0);
    return get_token(compiler_input_file);
}

Val this_inst_prim(Val *args, uint32_t num) {
    return (Val){TYPE_INT, {.int_data = this_inst()}};
}

Val next_inst_prim(Val *args, uint32_t num) {
    return (Val){TYPE_INT, {.int_data = next_inst()}};
}

Val set_const_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[args[0].int_data] = (Inst){INST_CONST, {.val = args[1]}};
    return (Val){TYPE_VOID};
}

Val set_var_prim(Val *args, uint32_t num) {
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

Val set_name_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_NAME, {.name = args[1].string_data}};
    return (Val){TYPE_VOID};
}

Val set_def_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_DEF, {.name = args[1].string_data}};
    return (Val){TYPE_VOID};
}

Val set_set_prim(Val *args, uint32_t num) {
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

Val set_set_name_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_SET_NAME, {.name = args[1].string_data}};
    return (Val){TYPE_VOID};
}

Val set_jump_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_JUMP, {.num = args[1].int_data}};
    return (Val){TYPE_VOID};
}

Val set_jump_false_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_JUMP_FALSE, {.num = args[1].int_data}};
    return (Val){TYPE_VOID};
}

Val set_lambda_prim(Val *args, uint32_t num) {
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

Val set_call_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_CALL, {.num = args[1].int_data}};
    return (Val){TYPE_VOID};
}

Val set_tail_call_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[args[0].int_data] = (Inst){INST_TAIL_CALL, {.num = args[1].int_data}};
    return (Val){TYPE_VOID};
}

Val set_return_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[args[0].int_data] = (Inst){INST_RETURN};
    return (Val){TYPE_VOID};
}

Val set_delete_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[args[0].int_data] = (Inst){INST_DELETE};
    return (Val){TYPE_VOID};
}

Val set_cons_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[args[0].int_data] = (Inst){INST_CONS};
    return (Val){TYPE_VOID};
}

uint32_t new_symbol_counter = 0;

// awful workaround for until macros are implemented
Val new_symbol_prim(Val *args, uint32_t num) {
    args_assert(num == 0);
    char *s = s_malloc(32);
    sprintf(s, "%d", new_symbol_counter++);
    return (Val){TYPE_SYMBOL, {.string_data = intern_symbol(s)}};
}
