#include <stdio.h>

#include "display.h"
#include "types.h"
#include "exec_stack.h"
#include "insts.h"

static void print_val_(Val val0, int display_style) {
    stack_push((Val){TYPE_PRINT_CONTROL, {.print_control_data = PRINT_CONTROL_END}});
    stack_push(val0);
    Val val;
    while (1) {
        val = stack_pop();
        switch(val.type) {
        case TYPE_INT:
            printf("%lld", val.int_data);
            break;
        case TYPE_FLOAT:
            printf("%.16g", val.float_data);
            if ((float)(int)val.float_data == val.float_data)
                putchar('.');
            break;
        case TYPE_BOOL:
            printf(val.int_data ? "#t" : "#f");
            break;
        case TYPE_CHAR: {
            printf("#\\");
            char32_t c = val.char_data;
            if (c < 0x80) {
                putchar((char)c);
            } else if (c < 0x800) {
                putchar((char)(0xC0 | (c >> 6)));
                putchar((char)(0x80 | (c & 0x3F)));
            } else if (c < 0x10000) {
                putchar((char)(0xE0 | (c >> 12)));
                putchar((char)(0x80 | ((c >> 6) & 0x3F)));
                putchar((char)(0x80 | (c & 0x3F)));
            } else {
                putchar((char)(0xF0 | (c >> 18)));
                putchar((char)(0x80 | ((c >> 12) & 0x3F)));
                putchar((char)(0x80 | ((c >> 6) & 0x3F)));
                putchar((char)(0x80 | (c & 0x3F)));
            }
            break;
        }
        case TYPE_STRING:
            printf(display_style ? "%s" : "\"%s\"", val.string_data);
            break;
        case TYPE_SYMBOL:
            printf("%s", val.string_data);
            break;
        case TYPE_PRIM:
        case TYPE_HIGH_PRIM:
            printf("<primitive procedure>");
            break;
        case TYPE_LAMBDA:
            printf("<lambda with arity %d%s>",
                    val.lambda_data->params & ~PARAMS_VARIADIC,
                    val.lambda_data->params & PARAMS_VARIADIC ? "+" : "");
            break;
        case TYPE_PAIR:
            printf("(");
            stack_push((Val){TYPE_PRINT_CONTROL, {.print_control_data = PRINT_CONTROL_END_LIST}});
            stack_push(val.pair_data->cdr);
            stack_push((Val){TYPE_PRINT_CONTROL, {.print_control_data = PRINT_CONTROL_CDR}});
            stack_push(val.pair_data->car);
            break;
        case TYPE_NIL:
            printf("()");
            break;
        case TYPE_VOID:
            printf("#!void");
            break;
        case TYPE_UNDEF:
            printf("#!undef");
            break;
        case TYPE_BROKEN_HEART:
            printf("</broken heart/>");
            break;
        case TYPE_ENV:
            printf("</environment at %p/>", val.env_data);
            break;
        case TYPE_INST:
            printf("</instruction pointer to %d/>", val.inst_data);
            break;
        case TYPE_GLOBAL_ENV:
            printf("</global environment at %p/>", val.global_env_data);
            break;
        case TYPE_PRINT_CONTROL:
            switch (val.print_control_data) {
            case PRINT_CONTROL_END:
                return;
            case PRINT_CONTROL_CDR:
                val = stack_pop();
                switch (val.type) {
                case TYPE_PAIR:
                    printf(" ");
                    stack_push(val.pair_data->cdr);
                    stack_push((Val){TYPE_PRINT_CONTROL, {.print_control_data = PRINT_CONTROL_CDR}});
                    stack_push(val.pair_data->car);
                    break;
                case TYPE_NIL:
                    break;
                default:
                    printf(" . ");
                    stack_push(val);
                    break;
                }
                break;
            case PRINT_CONTROL_END_LIST:
                printf(")");
                break;
            }
            break;
        }
    }
}

/* -- print_val
 * Prints a value as seen in the REPL.
 * Void type values are prevented from displaying on their own in `main`.
 * but they can still be printed as part of a compound data structure.
 */
void print_val(Val val) {
    print_val_(val, 0);
}

/* -- inner_display_val
 * Displays a value as seen as a result of the `display` primitive.
 * Differs from `print_val` in that is displays strings without quotes.
 */
void display_val(Val val) {
    print_val_(val, 1);
}

/* -- type_name
 * Returns a string containing a type name.
 * Used for error messages about invalid types.
 */
const char *type_name(Type type) {
    switch (type) {
    case TYPE_INT:
        return "int";
    case TYPE_FLOAT:
        return "float";
    case TYPE_BOOL:
        return "bool";
    case TYPE_CHAR:
        return "char";
    case TYPE_STRING:
        return "string";
    case TYPE_SYMBOL:
        return "symbol";
    case TYPE_PRIM:
    case TYPE_HIGH_PRIM:
        return "primitive";
    case TYPE_LAMBDA:
        return "lambda";
    case TYPE_PAIR:
        return "pair";
    case TYPE_NIL:
        return "nil";
    case TYPE_VOID:
        return "void";
    case TYPE_UNDEF:
        return "undef";
    case TYPE_BROKEN_HEART:
        return "/broken heart/";
    case TYPE_ENV:
        return "/environment/";
    case TYPE_INST:
        return "/instruction pointer/";
    case TYPE_GLOBAL_ENV:
        return "/global environment/";
    case TYPE_PRINT_CONTROL:
        return "/print control/";
    }
    return "//INVALID TYPE//";
}

void print_inst(uint32_t n) {
    printf("%d ", n);
    switch (insts[n].type) {
    case INST_CONST:
        printf("CONST ");
        print_val(insts[n].val);
        printf("\n");
        break;
    case INST_VAR:
        printf("VAR %d %d\n", insts[n].var.frame, insts[n].var.index);
        break;
    case INST_NAME:
        printf("NAME %s\n", insts[n].name);
        break;
    case INST_DEF:
        printf("DEF %s\n", insts[n].name);
        break;
    case INST_SET:
        printf("SET %d %d\n", insts[n].var.frame, insts[n].var.index);
        break;
    case INST_SET_NAME:
        printf("SET_NAME %s\n", insts[n].name);
        break;
    case INST_JUMP:
        printf("JUMP %d\n", insts[n].index);
        break;
    case INST_JUMP_FALSE:
        printf("JUMP_FALSE %d\n", insts[n].index);
        break;
    case INST_LAMBDA:
        printf("LAMBDA %d%s %d\n",
                insts[n].lambda.params & ~PARAMS_VARIADIC,
                insts[n].lambda.params & PARAMS_VARIADIC ? "+" : "",
                insts[n].lambda.index);
        break;
    case INST_CALL:
        printf("CALL %d\n", insts[n].num);
        break;
    case INST_TAIL_CALL:
        printf("TAIL_CALL %d\n", insts[n].num);
        break;
    case INST_RETURN:
        printf("RETURN\n");
        break;
    case INST_DELETE:
        printf("DELETE\n");
        break;
    case INST_CONS:
        printf("CONS\n");
        break;
    case INST_EXPR:
        printf("EXPR\n");
        break;
    case INST_EOF:
        printf("EOF\n");
        break;
    }
}
