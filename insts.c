#include <stdlib.h>
#include <string.h>

#include "insts.h"
#include "types.h"
#include "display.h"
#include "safestd.h"
#include "symbol.h"

Inst *insts;
uint32_t insts_size = 4096;
uint32_t inst_index = 0;

void setup_insts(void) {
    insts = s_malloc(insts_size * sizeof(Inst));
    return_inst = next_inst();
    insts[return_inst] = (Inst){INST_RETURN};
    tail_call_inst = next_inst();
    insts[tail_call_inst] = (Inst){INST_TAIL_CALL};
    compiler_pc = this_inst();
    load_insts(s_fopen("compiler.sss", "rb"));
    compile_pc = this_inst();
    insts[next_inst()] = (Inst){INST_NAME, {.name = "parse-and-compile"}};
    insts[next_inst()] = (Inst){INST_TAIL_CALL, {.num = 0}};
    parse_pc = this_inst();
    insts[next_inst()] = (Inst){INST_NAME, {.name = "parse"}};
    insts[next_inst()] = (Inst){INST_TAIL_CALL, {.num = 0}};
}

/* -- next_inst
 * Returns the next instruction in program memory and moves the allocation
 * pointer forward.
 */
uint32_t next_inst(void) {
    if (inst_index >= insts_size) {
        insts_size *= 2;
        insts = s_realloc(insts, insts_size * sizeof(Inst));
    }
    return inst_index++;
}

/* -- this_inst
 * Returns the next instruction in program memory without moving
 * the allocation pointer.
 */
uint32_t this_inst(void) {
    return inst_index;
}

/* -- next_expr
 * Finds the beginning of the next expression or end of code starting
 * from the given index.
 */
uint32_t next_expr(uint32_t start) {
    for (uint32_t i = start; ; i++)
        if (insts[i].type == INST_EXPR || insts[i].type == INST_EOF)
            return i;
}

void save_loc(FILE *fp, Env_loc loc) {
    for (int i = 1; i >= 0; i--)
        putc(loc.frame >> 8 * i, fp);
    for (int i = 1; i >= 0; i--)
        putc(loc.index >> 8 * i, fp);
}

void save_val(FILE *fp, Val val) {
    putc(val.type, fp);
    switch (val.type) {
    case TYPE_INT:
        for (int i = 7; i >= 0; i--)
            putc(val.int_data >> 8 * i, fp);
        break;
    case TYPE_FLOAT:
        // TODO
        break;
    case TYPE_BOOL:
        putc(val.int_data, fp);
        break;
    case TYPE_STRING:
    case TYPE_SYMBOL:
        fputs(val.string_data, fp);
        putc('\0', fp);
        break;
    case TYPE_NIL:
    case TYPE_VOID:
    case TYPE_UNDEF:
        break;
    default:
        eprintf("Error: value of type %s not a valid literal\n", type_name(val.type));
        exit(1);
    }
}

const char *magic = "\xf0\x9f\x91\xad""v3.0";

void save_magic(FILE *fp) {
    fputs(magic, fp);
}

void save_insts(FILE *fp, uint32_t start, uint32_t end) {
    for (uint32_t n = start; n != end; n++) {
        putc(insts[n].type, fp);
        switch (insts[n].type) {
        case INST_CONST:
            save_val(fp, insts[n].val);
            break;
        case INST_VAR:
        case INST_SET:
            save_loc(fp, insts[n].var);
            break;
        case INST_NAME:
        case INST_DEF:
        case INST_SET_NAME:
            fputs(insts[n].name, fp);
            putc('\0', fp);
            break;
        case INST_JUMP:
        case INST_JUMP_FALSE: {
            uint32_t index = insts[n].index - start;
            for (int i = 3; i >= 0; i--)
                putc(index >> 8 * i, fp);
            break;
        }
        case INST_LAMBDA: {
            for (int i = 3; i >= 0; i--)
                putc(insts[n].lambda.params >> 8 * i, fp);
            uint32_t index = insts[n].lambda.index - start;
            for (int i = 3; i >= 0; i--)
                putc(index >> 8 * i, fp);
            break;
        }
        case INST_CALL:
        case INST_TAIL_CALL:
            for (int i = 3; i >= 0; i--)
                putc(insts[n].num >> 8 * i, fp);
            break;
        case INST_RETURN:
        case INST_DELETE:
        case INST_CONS:
            break;
        }
    }
}

Env_loc load_loc(FILE *fp) {
    Env_loc loc = {0, 0};
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

Val load_val(FILE *fp) {
    char type = getc(fp);
    switch (type) {
    case TYPE_INT: {
        long long n = 0;
        for (int i = 0; i < 8; i++)
            n = n << 8 | getc(fp);
        return (Val){TYPE_INT, {.int_data = n}};
    }
    case TYPE_FLOAT:
        // TODO
        break;
    case TYPE_BOOL:
        return (Val){TYPE_BOOL, {.int_data = getc(fp)}};
    case TYPE_STRING:
        return (Val){TYPE_STRING, {.string_data = load_str(fp)}};
    case TYPE_SYMBOL:
        return (Val){TYPE_SYMBOL, {.string_data = intern_symbol(load_str(fp))}};
    case TYPE_NIL:
        return (Val){TYPE_NIL};
    case TYPE_VOID:
        return (Val){TYPE_VOID};
    case TYPE_UNDEF:
        return (Val){TYPE_UNDEF};
    default:
        eprintf("Error: invalid type (%d)\n", type);
        exit(1);
    }
}

void load_insts(FILE *fp) {
    uint32_t start = this_inst();
    char s[9];
    fgets(s, 9, fp);
    if (strcmp(s, magic) != 0) {
        eprintf("Error: not a valid bytecode file\n");
        exit(1);
    }
    char c;
    while ((c = getc(fp)) != EOF) {
        uint32_t n = next_inst();
        insts[n].type = c;
        switch (insts[n].type) {
        case INST_CONST:
            insts[n].val = load_val(fp);
            break;
        case INST_VAR:
        case INST_SET:
            insts[n].var = load_loc(fp);
            break;
        case INST_NAME:
        case INST_DEF:
        case INST_SET_NAME:
            insts[n].name = load_str(fp);
            break;
        case INST_JUMP:
        case INST_JUMP_FALSE: {
            uint32_t index = 0;
            for (int i = 0; i < 4; i++)
                index = index << 8 | getc(fp);
            insts[n].index = index + start;
            break;
        }
        case INST_LAMBDA: {
            uint32_t params = 0;
            for (int i = 0; i < 4; i++)
                params = params << 8 | getc(fp);
            insts[n].lambda.params = params;
            uint32_t index = 0;
            for (int i = 0; i < 4; i++)
                index = index << 8 | getc(fp);
            insts[n].lambda.index = index + start;
            break;
        }
        case INST_CALL:
        case INST_TAIL_CALL: {
            uint32_t num = 0;
            for (int i = 0; i < 4; i++)
                num = num << 8 | getc(fp);
            insts[n].num = num;
            break;
        }
        case INST_RETURN:
        case INST_DELETE:
        case INST_CONS:
        case INST_EXPR:
            break;
        default:
            eprintf("Error: invalid instruction type (%d)\n", insts[n].type);
            exit(1);
        }
    }
    insts[next_inst()] = (Inst){INST_EOF};
}
