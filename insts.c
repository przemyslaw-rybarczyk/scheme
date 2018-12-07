#include <stdlib.h>

#include "insts.h"
#include "expr.h"
#include "safemem.h"
#include "display.h"
#include "symbol.h"

struct inst *insts;
int insts_size = 4096;
int inst_index = 0;

int return_inst;

/* == insts.c
 * Allocates program memory to the compiler.
 * The program memory consists of a linked list of instruction arrays.
 * Each block ends in a BLOCK_JUMP instruction to give the illusion of continuous memory.
 */

void setup_insts(void) {
    insts = s_malloc(insts_size * sizeof(struct inst));
    return_inst = next_inst();
    insts[return_inst] = (struct inst){INST_RETURN};
    tail_call_inst = next_inst();
    insts[tail_call_inst] = (struct inst){INST_TAIL_CALL};
}

/* -- next_inst
 * Returns the next instruction in program memory and moves the allocation
 * pointer forward.
 */
int next_inst(void) {
    if (inst_index >= insts_size) {
        insts_size *= 2;
        insts = s_realloc(insts, insts_size * sizeof(struct inst));
    }
    return inst_index++;
}

/* -- this_inst
 * Returns the next instruction in program memory without moving
 * the allocation pointer.
 */
int this_inst(void) {
    return inst_index;
}

int next_expr(int start) {
    for (int i = start; ; i++)
        if (insts[i].type == INST_EXPR || insts[i].type == INST_EOF)
            return i;
}

void save_loc(FILE *fp, struct env_loc loc) {
    for (int i = 1; i >= 0; i--)
        putc(loc.frame >> 8 * i, fp);
    for (int i = 1; i >= 0; i--)
        putc(loc.index >> 8 * i, fp);
}

void save_val(FILE *fp, struct val val) {
    putc(val.type, fp);
    switch (val.type) {
    case TYPE_INT:
        for (int i = 7; i >= 0; i--)
            putc(val.data.int_data >> 8 * i, fp);
        break;
    case TYPE_FLOAT:
        // TODO
        break;
    case TYPE_BOOL:
        putc(val.data.int_data, fp);
        break;
    case TYPE_STRING:
    case TYPE_SYMBOL:
        fputs(val.data.string_data, fp);
        putc('\0', fp);
        break;
    case TYPE_NIL:
    case TYPE_VOID:
        break;
    default:
        fprintf(stderr, "Error: value of type %s not a valid literal\n", sprint_type(val.type));
        exit(3);
    }
}

void save_insts(FILE *fp, int start, int end) {
    for (int n = start; n != end; n++) {
        putc(insts[n].type, fp);
        switch (insts[n].type) {
        case INST_CONST:
            save_val(fp, insts[n].args.val);
            break;
        case INST_VAR:
        case INST_SET:
            save_loc(fp, insts[n].args.var);
            break;
        case INST_NAME:
        case INST_DEF:
        case INST_SET_NAME:
            fputs(insts[n].args.name, fp);
            putc('\0', fp);
            break;
        case INST_JUMP:
        case INST_JUMP_FALSE:
            for (int i = 3; i >= 0; i--)
                putc(insts[n].args.index >> 8 * i, fp);
            break;
        case INST_LAMBDA:
            for (int i = 3; i >= 0; i--)
                putc(insts[n].args.lambda.params >> 8 * i, fp);
            for (int i = 3; i >= 0; i--)
                putc(insts[n].args.lambda.index >> 8 * i, fp);
            break;
        case INST_CALL:
        case INST_TAIL_CALL:
            for (int i = 3; i >= 0; i--)
                putc(insts[n].args.num >> 8 * i, fp);
            break;
        case INST_RETURN:
        case INST_DELETE:
        case INST_CONS:
            break;
        }
    }
}

struct env_loc load_loc(FILE *fp) {
    struct env_loc loc = {0, 0};
    for (int i = 0; i < 2; i++)
        loc.frame = loc.frame << 8 | getc(fp);
    for (int i = 0; i < 2; i++)
        loc.index = loc.index << 8 | getc(fp);
    return loc;
}

char *load_str(FILE *fp) {
    int size = 16;
    char *str = s_malloc(size * sizeof(char));
    char *s = str;
    while ((*s++ = getc(fp)) != '\0') {
        if (s - str >= size) {
            size *= 2;
            int i = s - str;
            str = s_realloc(str, size * sizeof(char));
            s = str + i;
        }
    }
    return str;
}

struct val load_val(FILE *fp) {
    char type = getc(fp);
    switch (type) {
    case TYPE_INT: {
        long long n = 0;
        for (int i = 0; i < 8; i++)
            n = n << 8 | getc(fp);
        return (struct val){TYPE_INT, {.int_data = n}};
    }
    case TYPE_FLOAT:
        // TODO
        break;
    case TYPE_BOOL:
        return (struct val){TYPE_BOOL, {.int_data = getc(fp)}};
    case TYPE_STRING:
        return (struct val){TYPE_STRING, {.string_data = load_str(fp)}};
    case TYPE_SYMBOL:
        return (struct val){TYPE_SYMBOL, {.string_data = intern_symbol(load_str(fp))}};
    case TYPE_NIL:
        return (struct val){TYPE_NIL};
    case TYPE_VOID:
        return (struct val){TYPE_VOID};
    default:
        fprintf(stderr, "Error: invalid type (%d)\n", type);
        exit(3);
    }
}

void load_insts(FILE *fp) {
    char c;
    while ((c = getc(fp)) != EOF) {
        int n = next_inst();
        insts[n].type = c;
        switch (insts[n].type) {
        case INST_CONST:
            insts[n].args.val = load_val(fp);
            break;
        case INST_VAR:
        case INST_SET:
            insts[n].args.var = load_loc(fp);
            break;
        case INST_NAME:
        case INST_DEF:
        case INST_SET_NAME:
            insts[n].args.name = load_str(fp);
            break;
        case INST_JUMP:
        case INST_JUMP_FALSE: {
            int index = 0;
            for (int i = 0; i < 4; i++)
                index = index << 8 | getc(fp);
            insts[n].args.index = index;
            break;
        }
        case INST_LAMBDA: {
            int params = 0;
            for (int i = 0; i < 4; i++)
                params = params << 8 | getc(fp);
            insts[n].args.lambda.params = params;
            long long index = 0;
            for (int i = 0; i < 4; i++)
                index = index << 8 | getc(fp);
            insts[n].args.lambda.index = index;
            break;
        }
        case INST_CALL:
        case INST_TAIL_CALL: {
            int num = 0;
            for (int i = 0; i < 4; i++)
                num = num << 8 | getc(fp);
            insts[n].args.num = num;
            break;
        }
        case INST_RETURN:
        case INST_DELETE:
        case INST_CONS:
        case INST_EXPR:
            break;
        default:
            fprintf(stderr, "Error: invalid instruction type (%d)\n", insts[n].type);
            exit(3);
        }
    }
    insts[next_inst()] = (struct inst){INST_EOF};
}
