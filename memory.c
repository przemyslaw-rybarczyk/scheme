#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "types.h"
#include "env.h"
#include "exec.h"
#include "exec_gc.h"
#include "safestd.h"

void *mem_start;
size_t mem_size = 32 * 65536;
void *free_ptr;

void setup_memory(void) {
    free_ptr = mem_start = s_malloc(mem_size);
}

void garbage_collect();

/* -- env_lock
 * A pointer to an environment pointer which will be modified when
 * the environment moves during garbage collection.
 */
Env **env_lock = NULL;

void gc_lock_env(Env **env_ptr) {
    env_lock = env_ptr;
}

void gc_unlock_env(void) {
    env_lock = NULL;
}

/* -- gc_alloc
 * Allocates a given amount of memory and returns its address, possibly
 * invoking the garbage collector.
 */
void *gc_alloc(size_t size) {
#ifndef GC_ALWAYS
    if (free_ptr >= mem_start + mem_size - size)
#endif
        garbage_collect();
    if (free_ptr >= mem_start + mem_size - size) {
        printf("Internal error: out of memory\n");
        exit(1);
    }
    free_ptr += size;
    return free_ptr - size;
}

/* -- force_alloc
 * Allocates a given amount of memory and returns its address without checking
 * whether it can be allocated. It is used only inside the garbage collector,
 * as it's impossible to run out of memory there.
 */
void *force_alloc(size_t size) {
    free_ptr += size;
    return free_ptr - size;
}

Env *move_env(Env *env);
struct frame *move_frame(struct frame *frame);
Val move_val(Val val);
Pair *move_pair(Pair *pair);
Lambda *move_lambda(Lambda *lambda);

uint32_t call_count;

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
 * Each type of data is turned into a 'broken heart' after being moved to the
 * new heap.
 * +-----------+-------------------------------------+---------------------------+
 * | data type | conditon for broken heart           | field holding new address |
 * +-----------+-------------------------------------+---------------------------+
 * | Env       | env->size == UINT32_MAX             | env->outer                |
 * | Lambda    | lambda->body == UINT32_MAX          | lambda->new_ptr           |
 * | Pair      | pair->car.type == TYPE_BROKEN_HEART | pair->car.pair_data       |
 * +-----------+-------------------------------------+---------------------------+
 *
 * If a broken heart value is detected, the data has already been moved and the
 * moving function simply returns the address contained within it. Otherwise it
 * sets up the broken heart and copies data to the new address, determined by
 * the `free_ptr` pointer.
 */
void garbage_collect(void) {
    call_count = 0;
    void *new_mem = s_malloc(mem_size);
    free_ptr = new_mem;
    for (Binding *bind_ptr = execution_env->bindings; bind_ptr < execution_env->bindings + execution_env->size; bind_ptr++)
        bind_ptr->val = move_val(bind_ptr->val);
    for (Binding *bind_ptr = compiler_env->bindings; bind_ptr < compiler_env->bindings + compiler_env->size; bind_ptr++)
        bind_ptr->val = move_val(bind_ptr->val);
    for (Val *val_ptr = stack; val_ptr < stack_ptr; val_ptr++)
        *val_ptr = move_val(*val_ptr);
    exec_env = move_env(exec_env);
    if (env_lock != NULL)
        *env_lock = move_env(*env_lock);
    free(mem_start);
    mem_start = new_mem;
    if (free_ptr - mem_start >= mem_size / 2) {
        mem_size *= 2;
        garbage_collect();
    }
}

#define MAX_CALL_COUNT 262144

void increment_call_count(void) {
    if (call_count > MAX_CALL_COUNT) {
        eprintf("Error: recursion limit reached in garbage collector\n");
        exit(1);
    }
    call_count++;
}

void decrement_call_count(void) {
    call_count--;
}

Env *move_env(Env *env) {
    increment_call_count();
    if (env == NULL) {
        decrement_call_count();
        return env;
    }
    if (env->size == UINT32_MAX) {
        decrement_call_count();
        return env->outer;
    }
    Env *new_env = force_alloc(sizeof(Env) + env->size * sizeof(Val));
    *new_env = *env;
    env->size = UINT32_MAX;
    env->outer = new_env;
    new_env->outer = move_env(new_env->outer);
    for (size_t i = 0; i < new_env->size; i++)
        new_env->vals[i] = move_val(env->vals[i]);
    decrement_call_count();
    return new_env;
}

Val move_val(Val val) {
    increment_call_count();
    switch (val.type) {
    case TYPE_PAIR:
        val.pair_data = move_pair(val.pair_data);
        decrement_call_count();
        return val;
    case TYPE_LAMBDA:
        val.lambda_data = move_lambda(val.lambda_data);
        decrement_call_count();
        return val;
    case TYPE_ENV:
        val.env_data = move_env(val.env_data);
        decrement_call_count();
        return val;
    default:
        decrement_call_count();
        return val;
    }
}

Pair *move_pair(Pair *pair) {
    increment_call_count();
    if (pair->car.type == TYPE_BROKEN_HEART) {
        decrement_call_count();
        return pair->car.pair_data;
    }
    Pair *new_pair = force_alloc(sizeof(Pair));
    *new_pair = *pair;
    pair->car.type = TYPE_BROKEN_HEART;
    pair->car.pair_data = new_pair;
    new_pair->cdr = move_val(new_pair->cdr);
    new_pair->car = move_val(new_pair->car);
    decrement_call_count();
    return new_pair;
}

Lambda *move_lambda(Lambda *lambda) {
    increment_call_count();
    if (lambda->body == UINT32_MAX) {
        decrement_call_count();
        return lambda->new_ptr;
    }
    Lambda *new_lambda = force_alloc(sizeof(Lambda));
    *new_lambda = *lambda;
    lambda->body = UINT32_MAX;
    lambda->new_ptr = new_lambda;
    new_lambda->env = move_env(new_lambda->env);
    decrement_call_count();
    return new_lambda;
}
