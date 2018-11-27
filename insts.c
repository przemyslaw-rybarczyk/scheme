#include <stdlib.h>

#include "insts.h"
#include "expr.h"
#include "safemem.h"

#define INST_BLOCK_SIZE 4096

struct inst *inst_block_start = NULL;
struct inst *inst_end = NULL;

/* == insts.c
 * Allocates program memory to the compiler.
 * The program memory consists of a linked list of instruction arrays.
 * Each block ends in a JUMP instruction to give the illusion of continuous memory.
 */

void setup_insts(void) {
    inst_end = inst_block_start = s_malloc(INST_BLOCK_SIZE * sizeof(struct inst));
}

/* -- next_inst
 * Returns the next instruction in program memory and moves the allocation
 * pointer forward.
 */
struct inst *next_inst(void) {
    if (inst_end - inst_block_start >= INST_BLOCK_SIZE - 1) {
        inst_block_start = s_malloc(INST_BLOCK_SIZE * sizeof(struct inst));
        *inst_end = (struct inst){INST_JUMP, {.ptr = inst_block_start}};
        inst_end = inst_block_start;
    }
    return inst_end++;
}

/* -- this_inst
 * Returns the next instruction in program memory without moving
 * the allocation pointer.
 */
struct inst *this_inst(void) {
    return inst_end;
}
