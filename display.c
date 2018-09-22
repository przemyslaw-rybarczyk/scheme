#include <stdio.h>

#include "display.h"
#include "expr.h"

void display_param_list(struct param_list *params);
void display_val_list(struct pair *list);

/* -- display_val
 * Displays a value as seen in the REPL.
 * Void type values are prevented from displaying on their own in `main`.
 * but they can be displayed as part of lists.
 */
void display_val(struct val val) {
    switch(val.type) {
    case TYPE_INT:
        printf("%lld", val.data.int_data);
        break;
    case TYPE_FLOAT:
        printf("%.16g", val.data.float_data);
        if ((float)(int)val.data.float_data == val.data.float_data)
            putchar('.');
        break;
    case TYPE_BOOL:
        printf(val.data.int_data ? "#t" : "#f");
        break;
    case TYPE_STRING:
        printf("\"%s\"", val.data.string_data);
        break;
    case TYPE_SYMBOL:
        printf("%s", val.data.string_data);
        break;
    case TYPE_PRIM:
    case TYPE_HIGH_PRIM:
        printf("<primitive procedure>");
        break;
    case TYPE_LAMBDA:
        printf("<lambda with params (");
        display_param_list(val.data.lambda_data->params);
        printf(")>");
        break;
    case TYPE_PAIR:
        putchar('(');
        display_val_list(val.data.pair_data);
        putchar(')');
        break;
    case TYPE_NIL:
        printf("()");
        break;
    case TYPE_VOID:
        printf("<void>");
        break;
    case TYPE_BROKEN_HEART:
        printf("</broken heart/>");
        break;
    }
}

void display_param_list(struct param_list *params) {
    if (params == NULL)
        return;
    printf("%s", params->car);
    if (params->cdr != NULL)
        putchar(' ');
    display_param_list(params->cdr);
}

void display_val_list(struct pair *list) {
    display_val(list->car);
    switch (list->cdr.type) {
    case TYPE_PAIR:
        putchar(' ');
        display_val_list(list->cdr.data.pair_data);
        break;
    case TYPE_NIL:
        break;
    default:
        printf(" . ");
        display_val(list->cdr);
        break;
    }
}

void display_list_sexpr(struct sexpr_list *pair);

/* -- display_sexpr
 * Displays an sexpr.
 * Used for error messages about special forms with invalid syntax.
 */
void display_sexpr(struct sexpr *sexpr) {
    switch (sexpr->type) {
    case SEXPR_LITERAL:
        display_val(sexpr->data.literal);
        break;
    case SEXPR_ATOM:
        printf("%s", sexpr->data.atom);
        break;
    case SEXPR_CONS:
        putchar('(');
        display_list_sexpr(sexpr->data.cons);
        putchar(')');
        break;
    }
}

void display_list_sexpr(struct sexpr_list *pair) {
    if (pair == NULL)
        return;
    display_sexpr(pair->car);
    if (pair->cdr != NULL)
        putchar(' ');
    display_list_sexpr(pair->cdr);
}

/* -- sprint_type
 * Returns a string containing a type name.
 * Used for error messages about invalid types.
 */
const char *sprint_type(enum types type) {
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
    case TYPE_BROKEN_HEART:
        return "/broken heart/";
    }
}

void inner_display_val_list(struct pair *list);

/* -- inner_display_val
 * Displays a value as seen as a result of the `display` primitive.
 * Differs from `display_val` in that is displays string without quotes.
 */
void inner_display_val(struct val val) {
    switch (val.type) {
    case TYPE_STRING:
        printf("%s", val.data.string_data);
        break;
    case TYPE_PAIR:
        putchar('(');
        inner_display_val_list(val.data.pair_data);
        putchar(')');
        break;
    default:
        display_val(val);
        break;
    }
}

void inner_display_val_list(struct pair *list) {
    inner_display_val(list->car);
    switch (list->cdr.type) {
    case TYPE_PAIR:
        putchar(' ');
        inner_display_val_list(list->cdr.data.pair_data);
        break;
    case TYPE_NIL:
        break;
    default:
        printf(" . ");
        inner_display_val(list->cdr);
        break;
    }
}
