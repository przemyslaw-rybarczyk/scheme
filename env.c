#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "types.h"
#include "insts.h"
#include "memory.h"
#include "primitives.h"
#include "safestd.h"

/* -- compiler_env
 * The global environment in which the compiler is executed.
 */
Global_env *compiler_env;

/* -- make_global_env
 * Creates a global environment with the specified primitives.
 */
Global_env *make_global_env(int include_r5rs, int include_compiler) {
    Global_env *env = s_malloc(sizeof(Global_env));
    env->size = 0;
    if (include_r5rs)
        env->size += r5rs_bindings_size;
    if (include_compiler)
        env->size += compiler_bindings_size;
    env->capacity = 1;
    while (env->capacity < env->size)
        env->capacity *= 2;
    Binding *p = env->bindings = s_malloc(env->capacity * sizeof(Binding));
    if (include_r5rs) {
        memcpy(p, r5rs_bindings, r5rs_bindings_size * sizeof(Binding));
        p += r5rs_bindings_size;
    }
    if (include_compiler) {
        memcpy(p, compiler_bindings, compiler_bindings_size * sizeof(Binding));
        p += compiler_bindings_size;
    }
    return env;
}

void setup_env(void) {
    compiler_env = make_global_env(1, 1);
    for (uint32_t program = compiler_pc; insts[program].type != INST_EOF; program = next_expr(program + 1))
        exec(program, compiler_env);
}

/* -- locate_var
 * Finds a value bound to a location in a given environment.
 */
Val locate_var(Env_loc var, Env *env, Global_env *global) {
    if (var.frame == UINT32_MAX)
        return global->bindings[var.index].val;
    Env *frame = env;
    for (uint32_t i = var.frame; i > 0; i--)
        frame = frame->outer;
    return frame->vals[var.index];
}

/* -- locate_global_var
 * Finds a value with a given name in the global environment and returns its index.
 * Exits with an error if the name is unbound.
 */
uint32_t locate_global_var(char *var, Global_env *global) {
    for (uint32_t i = 0; i < global->size; i++)
        if (strcmp(global->bindings[i].var, var) == 0)
            return i;
    eprintf("Error: unbound variable %s\n", var);
    exit(1);
}

/* -- assign_var
 * Changes a value bound to a given location in a given environment.
 */
void assign_var(Env_loc var, Val val, Env *env, Global_env *global) {
    if (var.frame == UINT32_MAX) {
        global->bindings[var.index].val = val;
        return;
    }
    Env *frame = env;
    for (uint32_t n = var.frame; n > 0; n--)
        frame = frame->outer;
    frame->vals[var.index] = val;
}

/* -- define_var
 * Binds a value to a variable name in the given global environment.
 * Changes the binding if one already exists.
 */
void define_var(char* var, Val val, Global_env *global) {
    for (uint32_t i = 0; i < global->size; i++) {
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

/* -- extend_env
 * Extend an environment by a new frame containing bindings to the `vals_num`
 * values starting at `vals_start`.
 * Note: This function allocates garbage-collected data, so the environment
 * and values should be located on the stack when this function is called.
 */
Env *extend_env(Val *vals_start, uint32_t vals_num, Env *env) {
    Env *ext_env = gc_alloc(sizeof(Env) + vals_num * sizeof(Val));
    ext_env->outer = env;
    ext_env->size = vals_num;
    memcpy(ext_env->vals, vals_start, vals_num * sizeof(Val));
    return ext_env;
}
