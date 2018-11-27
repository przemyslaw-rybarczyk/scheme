#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "safemem.h"
#include "expr.h"
#include "env.h"
#include "exec.h"

union cell *mem_start;
size_t mem_size = 65536;
union cell *free_ptr;

struct env **env_stack[2];
struct env ***env_stack_ptr = env_stack;

/* -- setup_memory
 * Sets up the heap.
 * Should be called at the start of `main`.
 */
void setup_memory(void) {
    free_ptr = mem_start = s_malloc(mem_size * sizeof(union cell));
}

void garbage_collect();

/* -- alloc_cell
 * Allocates a memory cell and returns its address.
 * If the memory is full, it performs garbage collection.
 * This function is called by the more specific functions
 * `alloc_pair`, `alloc_lambda`, `alloc_frame`, `alloc_env`.
 * which allocate memory for specific types of cells by performing
 * the appropriate pointer conversions.
 */
union cell *alloc_cell(void) {
#ifndef GC_ALWAYS
    if (free_ptr - mem_start >= mem_size)
#endif
        garbage_collect();
    return free_ptr++;
}

struct pair *alloc_pair(void) {
    return &alloc_cell()->pair;
}

struct lambda *alloc_lambda(void) {
    return &alloc_cell()->lambda;
}

struct frame *alloc_frame(void) {
    return &alloc_cell()->frame;
}

struct env *alloc_env(void) {
    return &alloc_cell()->env;
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
 */
void garbage_collect(void) {
    // TODO try allocating less memory in case of malloc failure
    union cell *new_mem = s_malloc(mem_size * sizeof(union cell));
    free_ptr = new_mem;
    for (struct frame *frame = global_env_frame; frame != NULL; frame = frame->next)
        frame->binding.val = move_val(frame->binding.val);
    for (struct val *val_ptr = stack; val_ptr < stack_ptr; val_ptr++)
        *val_ptr = move_val(*val_ptr);
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
    if (env->new_ptr != NULL)
        return env->new_ptr;
    *free_ptr = (union cell){.env = *env};
    env->new_ptr = &free_ptr++->env;
    env = env->new_ptr;
    env->outer = move_env(env->outer);
    env->frame = move_frame(env->frame);
    return env;
}

struct frame *move_frame(struct frame *frame) {
    if (frame == NULL)
        return frame;
    if (frame->binding.val.type == TYPE_BROKEN_HEART)
        return frame->next;
    *free_ptr = (union cell){.frame = *frame};
    frame->binding.val.type = TYPE_BROKEN_HEART;
    frame->next = &free_ptr++->frame;
    frame = frame->next;
    frame->binding.val = move_val(frame->binding.val);
    frame->next = move_frame(frame->next);
    return frame;
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
        val.data.env_data = move_env(val.data.env_data);
        return val;
    default:
        return val;
    }
}

struct pair *move_pair(struct pair *pair) {
    if (pair->car.type == TYPE_BROKEN_HEART)
        return pair->car.data.pair_data;
    *free_ptr = (union cell){.pair = *pair};
    pair->car.type = TYPE_BROKEN_HEART;
    pair->car.data.pair_data = &free_ptr++->pair;
    pair = pair->car.data.pair_data;
    pair->cdr = move_val(pair->cdr);
    pair->car = move_val(pair->car);
    return pair;
}

struct lambda *move_lambda(struct lambda *lambda) {
    if (lambda->new_ptr != NULL)
        return lambda->new_ptr;
    *free_ptr = (union cell){.lambda = *lambda};
    lambda->new_ptr = &free_ptr++->lambda;
    lambda = lambda->new_ptr;
    lambda->env = move_env(lambda->env);
    return lambda;
}

void gc_push_env(struct env **env) {
    *env_stack_ptr++ = env;
}

void gc_pop_env() {
    --env_stack_ptr;
}
