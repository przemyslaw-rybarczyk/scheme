#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__) && !LOAD_FROM_CURRENT_DIR
#include <limits.h>
#endif

#include "insts.h"
#include "types.h"
#include "display.h"
#include "safestd.h"
#include "string.h"

Inst *insts;
static uint32_t insts_size = 4096;
static uint32_t inst_index = 0;

static char *get_path(void) {
#if defined(__linux__) && !LOAD_FROM_CURRENT_DIR
    char *path = realpath("/proc/self/exe", NULL);
    if (path == NULL)
        return "";
    char *last_separator = strrchr(path, '/');
    if (last_separator == NULL)
        return "";
    last_separator[1] = '\0';
    return path;
#else
    return "";
#endif
}

static FILE *fopen_relative(const char *dir, const char *name, const char *mode) {
    char *path = s_malloc(strlen(dir) + strlen(name) + 1);
    sprintf(path, "%s%s", dir, name);
    FILE *f = s_fopen(path, mode);
    free(path);
    return f;
}

High_prim map_prim_continuation;
High_prim for_each_prim_continuation;

void setup_insts(void) {
    insts = s_malloc(insts_size * sizeof(Inst));
    return_inst = next_inst();
    insts[return_inst] = (Inst){INST_RETURN};
    tail_call_inst = next_inst();
    insts[tail_call_inst] = (Inst){INST_TAIL_CALL};
    map_continue_inst = next_inst();
    insts[map_continue_inst] = (Inst){INST_CALL};
    insts[next_inst()] = (Inst){INST_CONST, {.val = (Val){TYPE_HIGH_PRIM, {.high_prim_data = map_prim_continuation}}}};
    insts[next_inst()] = (Inst){INST_TAIL_CALL, {.num = 0}};
    for_each_continue_inst = next_inst();
    insts[for_each_continue_inst] = (Inst){INST_CALL};
    insts[next_inst()] = (Inst){INST_CONST, {.val = (Val){TYPE_HIGH_PRIM, {.high_prim_data = for_each_prim_continuation}}}};
    insts[next_inst()] = (Inst){INST_TAIL_CALL, {.num = 0}};
    compiler_pc = this_inst();
    char *path = get_path();
    load_insts(fopen_relative(path, "compiler.sss", "rb"));
    compile_pc = this_inst();
    insts[next_inst()] = (Inst){INST_NAME, {.name = new_interned_string_from_cstring("parse-and-compile")}};
    insts[next_inst()] = (Inst){INST_TAIL_CALL, {.num = 0}};
    parse_pc = this_inst();
    insts[next_inst()] = (Inst){INST_NAME, {.name = new_interned_string_from_cstring("parse")}};
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

static void save_uint32(FILE *fp, uint32_t n) {
    for (int i = 3; i >= 0; i--)
        s_fputc((uint8_t)(n >> 8 * i), fp);
}

static void save_string(FILE *fp, String *str) {
    for (int i = 7; i >= 0; i--)
        s_fputc((uint8_t)(str->len >> 8 * i), fp);
    fputs32(str, fp);
}

static void save_val(FILE *fp, Val val) {
    s_fputc((int)val.type, fp);
    switch (val.type) {
    case TYPE_INT:
        for (int i = 7; i >= 0; i--)
            s_fputc((uint8_t)((uint64_t)val.int_data >> 8 * i), fp);
        break;
    case TYPE_FLOAT: {
        uint64_t n = (union {
            uint64_t i;
            double f;
        }) {.f = val.float_data}.i;
        for (int i = 7; i >= 0; i--)
            s_fputc((uint8_t)(n >> 8 * i), fp);
        break;
    }
    case TYPE_BOOL:
        s_fputc((uint8_t)val.int_data, fp);
        break;
    case TYPE_CONST_STRING:
    case TYPE_SYMBOL:
        save_string(fp, val.string_data);
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

static const char *magic = "\xf0\x9f\x91\xad";
static const char *version = "v4.0";

void save_insts(FILE *fp, uint32_t start, uint32_t end) {
    s_fputs(magic, fp);
    s_fputs(version, fp);
    for (uint32_t n = start; n != end; n++) {
        s_fputc((int)insts[n].type, fp);
        switch (insts[n].type) {
        case INST_CONST:
            save_val(fp, insts[n].val);
            break;
        case INST_VAR:
        case INST_SET:
            save_uint32(fp, insts[n].var.frame);
            save_uint32(fp, insts[n].var.index);
            break;
        case INST_NAME:
        case INST_DEF:
        case INST_SET_NAME:
            save_string(fp, insts[n].name);
            break;
        case INST_JUMP:
        case INST_JUMP_FALSE:
            save_uint32(fp, insts[n].index - start);
            break;
        case INST_LAMBDA: {
            save_uint32(fp, insts[n].lambda.params);
            save_uint32(fp, insts[n].lambda.index - start);
            break;
        }
        case INST_CALL:
        case INST_TAIL_CALL:
            save_uint32(fp, insts[n].num);
            break;
        case INST_RETURN:
        case INST_DELETE:
        case INST_CONS:
        case INST_EXPR:
        case INST_EOF:
            break;
        }
    }
}

static unsigned char s_fgetc2(FILE *f) {
    int c = s_fgetc(f);
    if (c == EOF) {
        eprintf("Error: unexpected end of file\n");
        exit(1);
    }
    return (unsigned char)c;
}

static char32_t s_fgetc32_2(FILE *f) {
    int32_t c = s_fgetc32(f);
    if (c == EOF32) {
        eprintf("Error: unexpected end of file\n");
        exit(1);
    }
    return (char32_t)c;
}

static uint32_t load_uint32(FILE *fp) {
    uint32_t n = 0;
    for (int i = 0; i < 4; i++)
        n = n << 8 | s_fgetc2(fp);
    return n;
}

static String *load_str(FILE *fp) {
    size_t len = 0;
    for (int i = 0; i < 8; i++)
        len = len << 8 | s_fgetc2(fp);
    String *str = s_malloc(sizeof(String) + len * sizeof(char32_t));
    str->len = len;
    for (size_t i = 0; i < len; i++)
        str->chars[i] = s_fgetc32_2(fp);
    return str;
}

static Val load_val(FILE *fp) {
    unsigned char type = s_fgetc2(fp);
    switch (type) {
    case TYPE_INT: {
        long long n = 0;
        for (int i = 0; i < 8; i++)
            n = n << 8 | s_fgetc2(fp);
        return (Val){TYPE_INT, {.int_data = n}};
    }
    case TYPE_FLOAT: {
        uint64_t n = 0;
        for (int i = 0; i < 8; i++)
            n = n << 8 | s_fgetc2(fp);
        double f = (union {
            uint64_t i;
            double f;
        }) {.i = n}.f;
        return (Val){TYPE_FLOAT, {.float_data = f}};
    }
    case TYPE_BOOL:
        return (Val){TYPE_BOOL, {.int_data = s_fgetc2(fp)}};
    case TYPE_CONST_STRING:
        return (Val){TYPE_CONST_STRING, {.string_data = load_str(fp)}};
    case TYPE_SYMBOL:
        return (Val){TYPE_SYMBOL, {.string_data = intern_string(load_str(fp))}};
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
    char s[5];
    s_fgets(s, 5, fp);
    if (strcmp(s, magic) != 0) {
        eprintf("Error: not a valid bytecode file\n");
        exit(1);
    }
    s_fgets(s, 5, fp);
    if (strcmp(s, version) != 0) {
        eprintf("Error: invalid bytecode file version %s\n", s);
        exit(1);
    }
    int c;
    while ((c = s_fgetc(fp)) != EOF) {
        uint32_t n = next_inst();
        insts[n].type = (enum Inst_type)c;
        switch (insts[n].type) {
        case INST_CONST:
            insts[n].val = load_val(fp);
            break;
        case INST_VAR:
        case INST_SET:
            insts[n].var.frame = load_uint32(fp);
            insts[n].var.index = load_uint32(fp);
            break;
        case INST_NAME:
        case INST_DEF:
        case INST_SET_NAME:
            insts[n].name = intern_string(load_str(fp));
            break;
        case INST_JUMP:
        case INST_JUMP_FALSE:
            insts[n].index = load_uint32(fp) + start;
            break;
        case INST_LAMBDA:
            insts[n].lambda.params = load_uint32(fp);
            insts[n].lambda.index = load_uint32(fp) + start;
            break;
        case INST_CALL:
        case INST_TAIL_CALL:
            insts[n].num = load_uint32(fp);
            break;
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
