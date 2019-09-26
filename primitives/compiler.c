#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"
#include "../types.h"
#include "../insts.h"
#include "../parser.h"
#include "../safestd.h"
#include "../symbol.h"
#include "assert.h"

static uint32_t u32_int_data(Val val) {
    if (val.int_data < 0 || val.int_data > UINT32_MAX) {
        eprintf("Internal compiler error: value %lld out of range\n", val.int_data);
        exit(1);
    }
    return (uint32_t)val.int_data;
}

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
    insts[u32_int_data(args[0])] = (Inst){INST_CONST, {.val = args[1]}};
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
    insts[u32_int_data(args[0])] = (Inst){INST_VAR, {.var = (Env_loc){u32_int_data(args[1]), u32_int_data(args[2])}}};
    return (Val){TYPE_VOID};
}

Val set_name_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[u32_int_data(args[0])] = (Inst){INST_NAME, {.name = args[1].string_data}};
    return (Val){TYPE_VOID};
}

Val set_def_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[u32_int_data(args[0])] = (Inst){INST_DEF, {.name = args[1].string_data}};
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
    insts[u32_int_data(args[0])] = (Inst){INST_SET, {.var = (Env_loc){u32_int_data(args[1]), u32_int_data(args[2])}}};
    return (Val){TYPE_VOID};
}

Val set_set_name_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_SYMBOL)
        type_error(args[1]);
    insts[u32_int_data(args[0])] = (Inst){INST_SET_NAME, {.name = args[1].string_data}};
    return (Val){TYPE_VOID};
}

Val set_jump_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[u32_int_data(args[0])] = (Inst){INST_JUMP, {.num = u32_int_data(args[1])}};
    return (Val){TYPE_VOID};
}

Val set_jump_false_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[u32_int_data(args[0])] = (Inst){INST_JUMP_FALSE, {.num = u32_int_data(args[1])}};
    return (Val){TYPE_VOID};
}

Val set_lambda_prim(Val *args, uint32_t num) {
    args_assert(num == 4);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_BOOL)
        type_error(args[1]);
    if (args[2].type != TYPE_INT)
        type_error(args[2]);
    if (args[3].type != TYPE_INT)
        type_error(args[3]);
    uint32_t params = u32_int_data(args[2]);
    if (u32_int_data(args[1]))
        params |= PARAMS_VARIADIC;
    insts[u32_int_data(args[0])] = (Inst){INST_LAMBDA, {.lambda = {params, u32_int_data(args[3])}}};
    return (Val){TYPE_VOID};
}

Val set_call_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[u32_int_data(args[0])] = (Inst){INST_CALL, {.num = u32_int_data(args[1])}};
    return (Val){TYPE_VOID};
}

Val set_tail_call_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    if (args[1].type != TYPE_INT)
        type_error(args[1]);
    insts[u32_int_data(args[0])] = (Inst){INST_TAIL_CALL, {.num = u32_int_data(args[1])}};
    return (Val){TYPE_VOID};
}

Val set_return_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[u32_int_data(args[0])] = (Inst){INST_RETURN};
    return (Val){TYPE_VOID};
}

Val set_delete_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[u32_int_data(args[0])] = (Inst){INST_DELETE};
    return (Val){TYPE_VOID};
}

Val set_cons_prim(Val *args, uint32_t num) {
    args_assert(num == 1);
    if (args[0].type != TYPE_INT)
        type_error(args[0]);
    insts[u32_int_data(args[0])] = (Inst){INST_CONS};
    return (Val){TYPE_VOID};
}
