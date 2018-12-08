#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "env.h"
#include "expr.h"
#include "safemem.h"
#include "primitives.h"
#include "memory.h"
#include "exec.h"

/* -- global_env
 * The frame containing the global environment.
 * Since the global environment is never garbage collected, this frame exists
 * outside of the heap.
 * The global environment itself is represented with a NULL pointer.
 * TODO redocument
 */
Val *global_env;
char **global_names;
int global_env_size, global_env_capacity;

/* -- setup_global_env
 * This function sets up the global environment by adding to it the primitives
 * according to the bindings in the `prims` and `high_prims` arrays.
 * It should be called at the start of the REPL.
 */
void setup_global_env(void) {
    global_env_size = global_env_capacity = prims_size + high_prims_size;
    global_env = s_malloc(global_env_capacity * sizeof(Val));
    global_names = s_malloc(global_env_capacity * sizeof(char *));
    for (int i = 0; i < prims_size; i++) {
        global_env[i] = (Val){TYPE_PRIM, {.prim_data = prims[i].val}};
        global_names[i] = prims[i].var;
    }
    for (int i = 0; i < high_prims_size; i++) {
        global_env[prims_size + i] = (Val){TYPE_HIGH_PRIM, {.high_prim_data = high_prims[i].val}};
        global_names[prims_size + i] = high_prims[i].var;
    }
}

/* -- locate_var
 * Finds a value bound to a location in a given environment.
 */
Val locate_var(Env_loc var, Env *env) {
    if (var.frame == -1)
        return global_env[var.index];
    Env *frame = env;
    for (int n = var.frame; n > 0; n--)
        frame = frame->outer;
    return frame->vals[var.index];
}

int locate_global_var(char *var) {
    for (int i = 0; i < global_env_size; i++)
        if (strcmp(global_names[i], var) == 0)
            return i;
//  for (int i = 0; i < global_env_size; i++)
//      printf("Global at %d is %s\n", i, global_names[i]);
    fprintf(stderr, "Error: unbound variable %s\n", var);
    exit(2);
}

/* -- assign_var
 * Changes a value bound to a variable name in a given environment.
 * Causes an error in case of an unbound variable.
 */
void assign_var(Env_loc var, Val val, Env *env) {
    if (var.frame == -1) {
//      printf("Assigning to global at %d\n", var.index);
        global_env[var.index] = val;
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
void define_var(char* var, Val val, Env *env) {
//  printf("Defining %s; global_env_size is %d\n", var, global_env_size);
    for (int i = 0; i < global_env_size; i++) {
        if (strcmp(global_names[i], var) == 0) {
            global_env[i] = val;
            return;
        }
    }
    if (global_env_size == global_env_capacity) {
        global_env_capacity *= 2;
        global_env = s_realloc(global_env, global_env_capacity * sizeof(Val));
        global_names = s_realloc(global_names, global_env_capacity * sizeof(char *));
//      printf("Expanded global env capacity to %d\n", global_env_capacity);
    }
    global_env[global_env_size] = val;
    global_names[global_env_size] = var;
//  printf("Defined %s at %d\n", var, global_env_size);
    global_env_size++;
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
