#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "types.h"
#include "env.h"
#include "exec.h"
#include "exec_gc.h"
#include "safestd.h"

static char *mem_start;
static size_t mem_size = 2097152;
static char *free_ptr;

/* -- GC_object
 * Contains a pointer to a pointer which is to be updated.
 */
typedef struct GC_object {
    enum {
        GC_VAL,
        GC_ENV,
    } type;
    union {
        Val *val;
        Env **env;
    };
} GC_object;

static GC_object *gc_stack;
static size_t gc_stack_size = 4096;
static GC_object *gc_stack_ptr;

static void gc_stack_push(GC_object obj) {
    if (gc_stack_ptr == gc_stack + gc_stack_size) {
        ptrdiff_t index = gc_stack_ptr - gc_stack;
        gc_stack_size *= 2;
        gc_stack = s_realloc(gc_stack, gc_stack_size * sizeof(GC_object));
        gc_stack_ptr = gc_stack + index;
    }
    *gc_stack_ptr++ = obj;
}

static GC_object gc_stack_pop(void) {
    return *--gc_stack_ptr;
}

void setup_memory(void) {
    free_ptr = mem_start = s_malloc(mem_size);
    gc_stack_ptr = gc_stack = s_malloc(gc_stack_size * sizeof(GC_object));
}

/* -- garbage_collect
 * Performs garbage collection and extends the heap size if more than half of it
 * is still taken up afterwards.
 *
 * The garbage collector is a simple stop-and-copy collector, which recursively
 * copies objects to the new memory, taking as a starting point the global
 * environment, evironments and values located on the stack, and `env_lock`.
 *
 * Unlike the garbage collector of SICP chapter 5.3.2, which uses a pointer that
 * sequentially scans all the copied cells, the copying here is performed
 * recursively, because memory constists of several data types, which can only
 * be known through the type of a pointer pointing to allocated data.
 *
 * In order to avoid a limit on recursion, the garbage collector is written
 * iteratively and uses a heap-allocated stack.
 *
 * Each type of data is turned into a 'broken heart' after being moved to the
 * new heap.
 * ┌───────────┬─────────────────────────────────────┬───────────────────────────┐
 * │ data type │ conditon for broken heart           │ field holding new address │
 * ├───────────┼─────────────────────────────────────┼───────────────────────────┤
 * │ Env       │ env->size == UINT32_MAX             │ env->outer                │
 * │ Lambda    │ lambda->body == UINT32_MAX          │ lambda->new_ptr           │
 * │ Pair      │ pair->car.type == TYPE_BROKEN_HEART │ pair->car.pair_data       │
 * │ String    │ str->chars[0] == UINT32_MAX         │ str->new_ptr              │
 * └───────────┴─────────────────────────────────────┴───────────────────────────┘
 *
 * If a broken heart value is detected, the data has already been moved and the
 * moving function simply returns the address contained within it. Otherwise it
 * sets up the broken heart and copies data to the new address, determined by
 * the `free_ptr` pointer.
 */
static void garbage_collect(void);

/* -- env_lock
 * A pointer to an environment pointer which will be modified when
 * the environment moves during garbage collection.
 */
static Env **env_lock = NULL;

void gc_lock_env(Env **env_ptr) {
    env_lock = env_ptr;
}

void gc_unlock_env(void) {
    env_lock = NULL;
}

static size_t align_size(size_t size) {
    return (size + 7) / 8 * 8;
}

/* -- gc_alloc
 * Allocates a given amount of memory and returns its address, possibly
 * invoking the garbage collector.
 */
void *gc_alloc(size_t size) {
    size = align_size(size);
#ifndef GC_ALWAYS
    if (free_ptr - mem_start >= (ptrdiff_t)mem_size - (ptrdiff_t)size)
#endif
        garbage_collect();
    if (free_ptr - mem_start >= (ptrdiff_t)mem_size - (ptrdiff_t)size) {
        mem_size *= 2;
        while (free_ptr - mem_start >= (ptrdiff_t)mem_size - (ptrdiff_t)size)
            mem_size *= 2;
        garbage_collect();
    }
    free_ptr += size;
    return free_ptr - size;
}

/* -- force_alloc
 * Allocates a given amount of memory and returns its address without checking
 * whether it can be allocated. It is used only inside the garbage collector,
 * as it's impossible to run out of memory there.
 */
static void *force_alloc(size_t size) {
    size = align_size(size);
    free_ptr += size;
    return free_ptr - size;
}

static Val move_val(Val val);
static Bigint *move_bigint(Bigint *bi);
static String *move_string(String *str);
static Vector *move_vector(Vector *vec);
static Env *move_env(Env *env);
static Pair *move_pair(Pair *pair);
static Lambda *move_lambda(Lambda *lambda);

static void garbage_collect(void) {
    void *new_mem = s_malloc(mem_size);
    free_ptr = new_mem;
    for (Binding *bind_ptr = execution_env->bindings; bind_ptr < execution_env->bindings + execution_env->size; bind_ptr++)
        gc_stack_push((GC_object){GC_VAL, {.val = &bind_ptr->val}});
    for (Binding *bind_ptr = compiler_env->bindings; bind_ptr < compiler_env->bindings + compiler_env->size; bind_ptr++)
        gc_stack_push((GC_object){GC_VAL, {.val = &bind_ptr->val}});
    for (Val *val_ptr = stack; val_ptr < stack_ptr; val_ptr++)
        gc_stack_push((GC_object){GC_VAL, {.val = val_ptr}});
    gc_stack_push((GC_object){GC_ENV, {.env = &exec_env}});
    if (env_lock != NULL)
        gc_stack_push((GC_object){GC_ENV, {.env = env_lock}});
    while (gc_stack_ptr != gc_stack) {
        GC_object obj = gc_stack_pop();
        switch (obj.type) {
        case GC_VAL:
            *obj.val = move_val(*obj.val);
            break;
        case GC_ENV:
            *obj.env = move_env(*obj.env);
            break;
        }
    }
    free(mem_start);
    mem_start = new_mem;
    // TODO leave extension for later
    if (free_ptr - mem_start >= mem_size / 2) {
        mem_size *= 2;
        garbage_collect();
    }
}

static Val move_val(Val val) {
    switch (val.type) {
    case TYPE_BIGINT:
        val.bigint_data = move_bigint(val.bigint_data);
        return val;
    case TYPE_STRING:
        val.string_data = move_string(val.string_data);
        return val;
    case TYPE_PAIR:
        val.pair_data = move_pair(val.pair_data);
        return val;
    case TYPE_VECTOR:
        val.vector_data = move_vector(val.vector_data);
        return val;
    case TYPE_LAMBDA:
        val.lambda_data = move_lambda(val.lambda_data);
        return val;
    case TYPE_ENV:
        val.env_data = move_env(val.env_data);
        return val;
    default:
        return val;
    }
}

static Bigint *move_bigint(Bigint *bi) {
    if (bi->len == PTRDIFF_MIN) {
        Bigint *new_ptr;
        memcpy(&new_ptr, bi->digits, sizeof(Bigint *));
        return new_ptr;
    }
    size_t digits_size = bilabs(bi->len) * sizeof(bi_base);
    size_t bi_size = sizeof(Bigint) + (digits_size > sizeof(Bigint *) ? digits_size : sizeof(Bigint *));
    Bigint *new_bi = force_alloc(bi_size);
    memcpy(new_bi, bi, bi_size);
    bi->len = PTRDIFF_MIN;
    memcpy(bi->digits, &new_bi, sizeof(Bigint *));
    return new_bi;
}

static String *move_string(String *str) {
    if (str->chars[0] == UINT32_MAX)
        return str->new_ptr;
    size_t str_size = sizeof(String) + (str->len ? str->len : 1) * sizeof(char32_t);
    String *new_str = force_alloc(str_size);
    memcpy(new_str, str, str_size);
    str->chars[0] = UINT32_MAX;
    str->new_ptr = new_str;
    return new_str;
}

static Vector *move_vector(Vector *vec) {
    if (vec->vals[0].type == TYPE_BROKEN_HEART)
        return vec->new_ptr;
    size_t vec_size = sizeof(Vector) + (vec->len ? vec->len : 1) * sizeof(Val);
    Vector *new_vec = force_alloc(vec_size);
    memcpy(new_vec, vec, vec_size);
    vec->vals[0].type = TYPE_BROKEN_HEART;
    vec->new_ptr = new_vec;
    for (size_t i = 0; i < new_vec->len; i++)
        gc_stack_push((GC_object){GC_VAL, {.val = &new_vec->vals[i]}});
    return new_vec;
}

static Env *move_env(Env *env) {
    if (env == NULL)
        return env;
    if (env->size == UINT32_MAX)
        return env->outer;
    Env *new_env = force_alloc(sizeof(Env) + env->size * sizeof(Val));
    memcpy(new_env, env, sizeof(Env) + env->size * sizeof(Val));
    env->size = UINT32_MAX;
    env->outer = new_env;
    gc_stack_push((GC_object){GC_ENV, {.env = &new_env->outer}});
    for (size_t i = 0; i < new_env->size; i++)
        gc_stack_push((GC_object){GC_VAL, {.val = &new_env->vals[i]}});
    return new_env;
}

static Pair *move_pair(Pair *pair) {
    if (pair->car.type == TYPE_BROKEN_HEART)
        return pair->car.pair_data;
    Pair *new_pair = force_alloc(sizeof(Pair));
    *new_pair = *pair;
    pair->car.type = TYPE_BROKEN_HEART;
    pair->car.pair_data = new_pair;
    gc_stack_push((GC_object){GC_VAL, {.val = &new_pair->cdr}});
    gc_stack_push((GC_object){GC_VAL, {.val = &new_pair->car}});
    return new_pair;
}

static Lambda *move_lambda(Lambda *lambda) {
    if (lambda->body == UINT32_MAX)
        return lambda->new_ptr;
    Lambda *new_lambda = force_alloc(sizeof(Lambda));
    *new_lambda = *lambda;
    lambda->body = UINT32_MAX;
    lambda->new_ptr = new_lambda;
    gc_stack_push((GC_object){GC_ENV, {.env = &new_lambda->env}});
    return new_lambda;
}
