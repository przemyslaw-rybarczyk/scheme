#include <stdio.h>

#include "display.h"
#include "types.h"
#include "exec_stack.h"
#include "insts.h"
#include "safestd.h"

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
        case TYPE_CHAR:
            printf("#\\");
            putchar32(val.char_data);
            break;
        case TYPE_STRING:
        case TYPE_CONST_STRING:
            if (display_style == 0)
                printf("\"");
            puts32(val.string_data);
            if (display_style == 0)
                printf("\"");
            break;
        case TYPE_SYMBOL:
            puts32(val.string_data);
            break;
        case TYPE_PRIM:
        case TYPE_HIGH_PRIM:
            printf("<primitive procedure>");
            break;
        case TYPE_LAMBDA:
            printf("<lambda with arity %"PRIu32"%s>",
                    val.lambda_data->params & ~PARAMS_VARIADIC,
                    val.lambda_data->params & PARAMS_VARIADIC ? "+" : "");
            break;
        case TYPE_PAIR:
        case TYPE_CONST_PAIR:
            printf("(");
            stack_push((Val){TYPE_PRINT_CONTROL, {.print_control_data = PRINT_CONTROL_END_LIST}});
            stack_push(val.pair_data->cdr);
            stack_push((Val){TYPE_PRINT_CONTROL, {.print_control_data = PRINT_CONTROL_CDR}});
            stack_push(val.pair_data->car);
            break;
        case TYPE_VECTOR:
        case TYPE_CONST_VECTOR:
            printf("#(");
            stack_push((Val){TYPE_PRINT_CONTROL, {.print_control_data = PRINT_CONTROL_END_LIST}});
            for (size_t i = val.vector_data->len; i-- > 0; ) {
                stack_push(val.vector_data->vals[i]);
                if (i != 0)
                    stack_push((Val){TYPE_PRINT_CONTROL, {.print_control_data = PRINT_CONTROL_SPACE}});
            }
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
            printf("</instruction pointer to %"PRIu32"/>", val.inst_data);
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
                case TYPE_CONST_PAIR:
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
            case PRINT_CONTROL_SPACE:
                printf(" ");
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

/* -- display_val
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
    case TYPE_CONST_STRING:
        return "string";
    case TYPE_SYMBOL:
        return "symbol";
    case TYPE_PRIM:
    case TYPE_HIGH_PRIM:
        return "primitive";
    case TYPE_LAMBDA:
        return "lambda";
    case TYPE_PAIR:
    case TYPE_CONST_PAIR:
        return "pair";
    case TYPE_VECTOR:
    case TYPE_CONST_VECTOR:
        return "vector";
        break;
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
    printf("%"PRIu32" ", n);
    switch (insts[n].type) {
    case INST_CONST:
        printf("CONST ");
        print_val(insts[n].val);
        printf("\n");
        break;
    case INST_VAR:
        printf("VAR %"PRIu32" %"PRIu32"\n", insts[n].var.frame, insts[n].var.index);
        break;
    case INST_NAME:
        printf("NAME ");
        puts32(insts[n].name);
        printf("\n");
        break;
    case INST_DEF:
        printf("DEF ");
        puts32(insts[n].name);
        printf("\n");
        break;
    case INST_SET:
        printf("SET %"PRIu32" %"PRIu32"\n", insts[n].var.frame, insts[n].var.index);
        break;
    case INST_SET_NAME:
        printf("SET_NAME ");
        puts32(insts[n].name);
        printf("\n");
        break;
    case INST_JUMP:
        printf("JUMP %"PRIu32"\n", insts[n].index);
        break;
    case INST_JUMP_FALSE:
        printf("JUMP_FALSE %"PRIu32"\n", insts[n].index);
        break;
    case INST_LAMBDA:
        printf("LAMBDA %"PRIu32"%s %"PRIu32"\n",
                insts[n].lambda.params & ~PARAMS_VARIADIC,
                insts[n].lambda.params & PARAMS_VARIADIC ? "+" : "",
                insts[n].lambda.index);
        break;
    case INST_CALL:
        printf("CALL %"PRIu32"\n", insts[n].num);
        break;
    case INST_TAIL_CALL:
        printf("TAIL_CALL %"PRIu32"\n", insts[n].num);
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
