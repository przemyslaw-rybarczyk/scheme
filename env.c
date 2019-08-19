#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "env.h"
#include "expr.h"
#include "safemem.h"
#include "primitives.h"
#include "memory.h"
#include "exec.h"
#include "primitives/compiler.h"
#include "insts.h"

// TODO REWRITE

Global_env *compiler_env;

Global_env *make_global_env(void) {
    int size = prims_size + high_prims_size;
    Global_env *env = s_malloc(sizeof(Global_env));
    env->size = env->capacity = size;
    env->bindings = s_malloc(sizeof(Binding) * size);
    for (int i = 0; i < prims_size; i++)
        env->bindings[i] = (Binding){{TYPE_PRIM, {.prim_data = prims[i].val}}, prims[i].var};
    for (int i = 0; i < high_prims_size; i++)
        env->bindings[prims_size + i] = (Binding){{TYPE_HIGH_PRIM, {.high_prim_data = high_prims[i].val}}, high_prims[i].var};
    return env;
}

Global_env *make_compile_env(void) {
    int size = prims_size + high_prims_size + compiler_prims_size;
    Global_env *env = s_malloc(sizeof(Global_env));
    env->size = env->capacity = size;
    env->bindings = s_malloc(sizeof(Binding) * size);
    for (int i = 0; i < prims_size; i++)
        env->bindings[i] = (Binding){{TYPE_PRIM, {.prim_data = prims[i].val}}, prims[i].var};
    size_t d = prims_size;
    for (int i = 0; i < high_prims_size; i++)
        env->bindings[d + i] = (Binding){{TYPE_HIGH_PRIM, {.high_prim_data = high_prims[i].val}}, high_prims[i].var};
    d += high_prims_size;
    for (int i = 0; i < compiler_prims_size; i++)
        env->bindings[d + i] = (Binding){{TYPE_PRIM, {.prim_data = compiler_prims[i].val}}, compiler_prims[i].var};
    return env;
}

void setup_env(void) {
    compiler_env = make_compile_env();
    for (int program = compiler_pc; insts[program].type != INST_EOF; program = next_expr(program + 1)) {
        change_global_env(compiler_env);
        exec(program);
    }
}

/* -- locate_var
 * Finds a value bound to a location in a given environment.
 */
Val locate_var(Env_loc var, Env *env, Global_env *global) {
    if (var.frame == -1)
        return global->bindings[var.index].val;
    Env *frame = env;
    for (int n = var.frame; n > 0; n--)
        frame = frame->outer;
    return frame->vals[var.index];
}

int locate_global_var(char *var, Global_env *global) {
    for (int i = 0; i < global->size; i++)
        if (strcmp(global->bindings[i].var, var) == 0)
            return i;
    fprintf(stderr, "Error: unbound variable %s\n", var);
    exit(2);
}

/* -- assign_var
 * Changes a value bound to a variable name in a given environment.
 * Causes an error in case of an unbound variable.
 */
void assign_var(Env_loc var, Val val, Env *env, Global_env *global) {
    if (var.frame == -1) {
        global->bindings[var.index].val = val;
        return;
    }
    Env *frame = env;
    for (int n = var.frame; n > 0; n--)
        frame = frame->outer;
    frame->vals[var.index] = val;
}

/* -- define_var
 * Binds a value to a variable name in the first frame of a given environment.
 * Changes the binding if one already exists in the frame.
 */
void define_var(char* var, Val val, Global_env *global) {
    for (int i = 0; i < global->size; i++) {
        if (strcmp(global->bindings[i].var, var) == 0) {
            global->bindings[i].val = val;
            return;
        }
    }
    if (global->size == global->capacity) {
        global->capacity *= 2;
        global->bindings = s_realloc(global->bindings, global->capacity * sizeof(Binding));
    }
    global->bindings[global->size++] = (Binding){val, var};
}

void argnum_assert(int assertion) {
    if (!assertion) {
        fprintf(stderr, "Invalid number of arguments to lambda\n");
        exit(2);
    }
}

/* -- extend_env
 * Extend an environment by a new frame containing the variables is `vars`
 * bound to the `vals_num` variables in an array starting at `vals_start`.
 */
Env *extend_env(Val *vals_start, int vals_num, Env *env) {
    gc_push_env(&env);
    Env *ext_env = gc_alloc(sizeof(Env) + vals_num * sizeof(Val));
    ext_env->outer = env;
    ext_env->size = vals_num;
    memcpy(ext_env->vals, vals_start, vals_num * sizeof(Val));
    gc_pop_env();
    return ext_env;
}
