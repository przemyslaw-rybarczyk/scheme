#include <stdio.h>

#include "display.h"
#include "types.h"
#include "insts.h"

void print_val_list(Pair *list);

/* -- print_val
 * Prints a value as seen in the REPL.
 * Void type values are prevented from displaying on their own in `main`.
 * but they can still be printed as part of a compound data structure.
 */
void print_val(Val val) {
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
    case TYPE_STRING:
        printf("\"%s\"", val.string_data);
        break;
    case TYPE_SYMBOL:
        printf("%s", val.string_data);
        break;
    case TYPE_PRIM:
    case TYPE_HIGH_PRIM:
        printf("<primitive procedure>");
        break;
    case TYPE_LAMBDA:
        printf("<lambda with arity %d>", val.lambda_data->params);
        break;
    case TYPE_PAIR:
        putchar('(');
        print_val_list(val.pair_data);
        putchar(')');
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
    }
}

void print_val_list(Pair *list) {
    print_val(list->car);
    switch (list->cdr.type) {
    case TYPE_PAIR:
        putchar(' ');
        print_val_list(list->cdr.pair_data);
        break;
    case TYPE_NIL:
        break;
    default:
        printf(" . ");
        print_val(list->cdr);
        break;
    }
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
    }
}

void display_val_list(Pair *list);

/* -- inner_display_val
 * Displays a value as seen as a result of the `display` primitive.
 * Differs from `print_val` in that is displays string without quotes.
 */
void display_val(Val val) {
    switch (val.type) {
    case TYPE_STRING:
        printf("%s", val.string_data);
        break;
    case TYPE_PAIR:
        putchar('(');
        display_val_list(val.pair_data);
        putchar(')');
        break;
    default:
        print_val(val);
        break;
    }
}

void display_val_list(Pair *list) {
    display_val(list->car);
    switch (list->cdr.type) {
    case TYPE_PAIR:
        putchar(' ');
        display_val_list(list->cdr.pair_data);
        break;
    case TYPE_NIL:
        break;
    default:
        printf(" . ");
        display_val(list->cdr);
        break;
    }
}

void print_inst(int n) {
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
        printf("LAMBDA %d %d\n", insts[n].lambda.params, insts[n].lambda.index);
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
