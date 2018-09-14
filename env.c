#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "env.h"
#include "expr.h"
#include "safemem.h"
#include "primitives.h"
#include "memory.h"

/* -- global_env_frame
 * The frame containing the global environment.
 * Since the global environment is never garbage collected, this frame exists
 * outside of the heap.
 * The global environment itself is represented with a NULL pointer.
 */
struct frame *global_env_frame = NULL;

/* -- setup_global_env
 * This function sets up the global environment by adding to it the primitives
 * contained in the `prim_vals` array bound to the names in the `prim_vars` array.
 * It should be called at the start of the REPL.
 */
void setup_global_env() {
    struct val (**val)(struct val_list *) = prim_vals;
    char **var = prim_vars;
    while (val < prim_vals + prims && var < prim_vars + prims) {
        struct frame *frame = s_malloc(sizeof(struct frame));
        frame->binding = (struct binding){{TYPE_PRIM, {.prim_data = *val}}, *var};
        frame->next = global_env_frame;
        global_env_frame = frame;
        val++;
        var++;
    }
}

/* -- lookup_var
 * Finds a value bound to a variable name in a given environment.
 * Causes an error in case of an unbound variable.
 */
struct val lookup_var(char *var, struct env *env) {
    while (env != NULL) {
        struct frame *frame = env->frame;
        while (frame != NULL) {
            if (strcmp(var, frame->binding.var) == 0)
                return frame->binding.val;
            frame = frame->next;
        }
        env = env->outer;
    }
    struct frame *frame = global_env_frame;
    while (frame != NULL) {
        if (strcmp(var, frame->binding.var) == 0)
            return frame->binding.val;
        frame = frame->next;
    }
    fprintf(stderr, "Error: unbound variable - %s\n", var);
    exit(1);
}

/* -- set_var
 * Changes a value bound to a variable name in a given environment.
 * Causes an error in case of an unbound variable.
 */
struct val assign_var(char *var, struct val val, struct env *env) {
    while (env != NULL) {
        struct frame *frame = env->frame;
        while (frame != NULL) {
            if (strcmp(var, frame->binding.var) == 0) {
                frame->binding.val = val;
                return (struct val){TYPE_VOID};
            }
            frame = frame->next;
        }
        env = env->outer;
    }
    struct frame *frame = global_env_frame;
    while (frame != NULL) {
        if (strcmp(var, frame->binding.var) == 0) {
            frame->binding.val = val;
            return (struct val){TYPE_VOID};
        }
        frame = frame->next;
    }
    fprintf(stderr, "Error: unbound variable - %s\n", var);
    exit(1);
}

/* -- define_var
 * Binds a value to a variable name in the first frame of a given environment.
 * Changes the binding if one already exists in the frame.
 */
struct val define_var(char* var, struct val val, struct env *env) {
    struct frame *env_frame = env ? env->frame : global_env_frame;
    struct frame *frame = env_frame;
    while (frame != NULL) {
        if (strcmp(var, frame->binding.var) == 0) {
            frame->binding.val = val;
            return (struct val){TYPE_VOID};
        }
        frame = frame->next;
    }
    struct frame *new_frame;
    if (env != NULL) {
        gc_push_env(&env);
        gc_push_val(&val);
        new_frame = alloc_frame();
        gc_pop_val();
        gc_pop_env();
    } else
        new_frame = s_malloc(sizeof(struct frame));
    new_frame->binding.val = val;
    new_frame->binding.var = var;
    new_frame->next = env_frame;
    if (env != NULL)
        env->frame = new_frame;
    else
        global_env_frame = new_frame;
    return (struct val){TYPE_VOID};
}

void argnum_assert(int assertion) {
    if (!assertion) {
        fprintf(stderr, "Invalid number of arguments to lambda\n");
        exit(2);
    }
}

/* -- extend_env
 * Extend an environment by a new frame containing the variables is `vars`
 * bound to the values in `vals`.
 */
struct env *extend_env(struct param_list *vars, struct val_list *vals, struct env *env) {
    if (vars == NULL && vals == NULL)
        return env;
    argnum_assert(vars != NULL && vals != NULL);
    gc_push_env(&env);
    struct env *new_env = alloc_env();
    gc_push_env(&new_env);
    new_env->frame = NULL;
    new_env->outer = env;
    new_env->new_ptr = NULL;
    struct frame *frame = alloc_frame();
    while (vars->cdr != NULL && vals->cdr != NULL) {
        frame->binding.var = vars->car;
        frame->binding.val = vals->car;
        frame->next = new_env->frame;
        new_env->frame = frame;
        vars = vars->cdr;
        vals = vals->cdr;
        frame = alloc_frame();
    }
    argnum_assert(vars->cdr == NULL && vars->cdr == NULL);
    frame->binding.var = vars->car;
    frame->binding.val = vals->car;
    frame->next = new_env->frame;
    new_env->frame = frame;
    gc_pop_env();
    gc_pop_env();
    return new_env;
}
