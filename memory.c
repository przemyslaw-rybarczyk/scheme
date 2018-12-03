#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "safemem.h"
#include "expr.h"
#include "env.h"
#include "exec.h"

void *mem_start;
size_t mem_size = 32 * 65536;
void *free_ptr;

struct env **env_stack[2];
struct env ***env_stack_ptr = env_stack;

/* -- setup_memory
 * Sets up the heap.
 * Should be called at the start of `main`.
 */
void setup_memory(void) {
    free_ptr = mem_start = s_malloc(mem_size);
}

void garbage_collect();

/* -- gc_alloc
 * TODO doc
 * Allocates a memory cell and returns its address.
 * If the memory is full, it performs garbage collection.
 * This function is called by the more specific functions
 * `alloc_pair`, `alloc_lambda`, `alloc_frame`, `alloc_env`.
 * which allocate memory for specific types of cells by performing
 * the appropriate pointer conversions.
 */
void *gc_alloc(size_t size) {
#ifndef GC_ALWAYS
    if (free_ptr + size >= mem_start + mem_size)
#endif
        garbage_collect();
    if (free_ptr + size >= mem_start + mem_size) {
        // TODO expand memory
        printf("Internal error: out of memory\n");
        exit(-1);
    }
    free_ptr += size;
    return free_ptr - size;
}

/* -- force_alloc
 */
void *force_alloc(size_t size) {
    free_ptr += size;
    return free_ptr - size;
}

struct env *move_env(struct env *env);
struct frame *move_frame(struct frame *frame);
struct val move_val(struct val val);
struct pair *move_pair(struct pair *pair);
struct lambda *move_lambda(struct lambda *lambda);

/* -- garbage_collect
 * Performs garbage collection and extends the heap size if more than half of it
 * is still taken up afterwards.
 *
 * The garbage collector is a simple stop-and-copy collector, which recursively
 * copies cells to the new memory, taking as a starting point the global
 * environment, evironments and values located on the VM stack, and `env_stack`,
 * a special stack containing pointers to up to two environment pointers, used
 * in some environment-related functions.
 *
 * Unlike the garbage collector of SICP chapter 5.3.2, which uses a pointer that
 * sequentially scans all the copied cells, the copying here is performed
 * recursively, because cells can contain one of four data types, which can only
 * be known through the type of a pointer pointing to data allocated within
 * the cell.
 *
 * Each type of cell is turned into a 'broken heart' after being moved to the
 * new heap. For `env`s and `lambda`s this is indicated through setting a special
 * `new_ptr` member variable to point to the reallocated data. The `frame`s and
 * `pair`s have their `binding.val` and `car`, respectively, member's `type` set
 * to a special value of TYPE_BROKEN_HEART, with the pointer respectively in
 * `next` and `car.data.pair_data` members.
 *
 * If a broken heart value is detected, the cell has already been moved and the
 * moving function simply returns the address contained within it. Otherwise it
 * sets up the broken heart and copies data to the new address, determined by
 * the `free_ptr` pointer.
 *
 * TODO redocument
 */
void garbage_collect(void) {
    // TODO try allocating less memory in case of malloc failure
    void *new_mem = s_malloc(mem_size);
    free_ptr = new_mem;
    for (struct val *val_ptr = global_env; val_ptr < global_env + global_env_size; val_ptr++)
        *val_ptr = move_val(*val_ptr);
//  printf("Stack ptr is %p\n", stack_ptr);
    for (struct val *val_ptr = stack; val_ptr < stack_ptr; val_ptr++) {
//      printf("Moving val at %p\n", val_ptr);
        *val_ptr = move_val(*val_ptr);
    }
    exec_env = move_env(exec_env);
    for (struct env ***env_ptr = env_stack; env_ptr < env_stack_ptr; env_ptr++)
        **env_ptr = move_env(**env_ptr);
    free(mem_start);
    mem_start = new_mem;
    if (free_ptr - mem_start >= mem_size / 2) {
        mem_size *= 2;
        garbage_collect();
    }
}

struct env *move_env(struct env *env) {
    if (env == NULL)
        return env;
    if (env->size == -1)
        return env->outer;
    struct env *new_env = force_alloc(sizeof(struct env) + env->size * sizeof(struct val));
    *new_env = *env;
    env->size = -1;
    env->outer = new_env;
    new_env->outer = move_env(new_env->outer);
    for (size_t i = 0; i < new_env->size; i++)
        new_env->vals[i] = move_val(env->vals[i]);
//  printf("Moved to %p\n", new_env);
    return new_env;
}

struct val move_val(struct val val) {
    switch (val.type) {
    case TYPE_PAIR:
        val.data.pair_data = move_pair(val.data.pair_data);
        return val;
    case TYPE_LAMBDA:
        val.data.lambda_data = move_lambda(val.data.lambda_data);
        return val;
    case TYPE_ENV:
//      printf("Will move memory from %p\n", val.data.env_data);
        val.data.env_data = move_env(val.data.env_data);
        return val;
    default:
        return val;
    }
}

struct pair *move_pair(struct pair *pair) {
    if (pair->car.type == TYPE_BROKEN_HEART)
        return pair->car.data.pair_data;
    struct pair *new_pair = force_alloc(sizeof(struct pair));
    *new_pair = *pair;
    pair->car.type = TYPE_BROKEN_HEART;
    pair->car.data.pair_data = new_pair;
    new_pair->cdr = move_val(new_pair->cdr);
    new_pair->car = move_val(new_pair->car);
    return new_pair;
}

struct lambda *move_lambda(struct lambda *lambda) {
    if (lambda->new_ptr != NULL)
        return lambda->new_ptr;
    struct lambda *new_lambda = force_alloc(sizeof(struct lambda));
    *new_lambda = *lambda;
    lambda->new_ptr = new_lambda;
    new_lambda->env = move_env(new_lambda->env);
    return new_lambda;
}

void gc_push_env(struct env **env) {
    *env_stack_ptr++ = env;
}

void gc_pop_env() {
    --env_stack_ptr;
}
