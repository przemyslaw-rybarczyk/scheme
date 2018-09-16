#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "display.h"
#include "memory.h"
#include "eval.h"

/* == primitives.c
 * This file includes all primitives included in this Scheme implementation.
 * Primitives are represented as functions from `struct val_list *` to `struct val`.
 * They are listed in the `prim_vals` array at the bottom of the file,
 * along with the names they're bound to in the `prim_vals` array.
 */

void type_error(struct val val) {
    fprintf(stderr, "Error: incorrect argument type - %s\n",
            sprint_type(val.type));
    exit(2);
}

void args_error() {
    fprintf(stderr, "Error: invalid number of arguments\n");
    exit(2);
}

struct val add_prim(struct val_list *args) {
    long long int_sum = 0;
    double float_sum;
    int float_val = 0;
    while (args != NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            if (float_val)
                float_sum += (double)args->car.data.int_data;
            else
                int_sum += args->car.data.int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_sum = (double)int_sum;
                float_val = 1;
            }
            float_sum += args->car.data.float_data;
            break;
        default:
            type_error(args->car);
        }
        args = args->cdr;
    }
    if (float_val)
        return (struct val){TYPE_FLOAT, {.float_data = float_sum}};
    else
        return (struct val){TYPE_INT, {.int_data = int_sum}};
}

struct val sub_prim(struct val_list *args) {
    long long int_diff;
    double float_diff;
    int float_val = 0;
    if (args == NULL) {
        fprintf(stderr, "Error: no arguments");
        exit(2);
    }
    if (args->cdr == NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            return (struct val){TYPE_INT, {.int_data = - args->car.data.int_data}};
        case TYPE_FLOAT:
            return (struct val){TYPE_FLOAT, {.float_data = - args->car.data.float_data}};
        default:
            type_error(args->car);
        }
    }
    switch (args->car.type) {
    case TYPE_INT:
        int_diff = args->car.data.int_data;
        break;
    case TYPE_FLOAT:
        float_diff = args->car.data.float_data;
        float_val = 1;
        break;
    default:
        type_error(args->car);
    }
    args = args->cdr;
    while (args != NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            if (float_val)
                float_diff -= (double)args->car.data.int_data;
            else
                int_diff -= args->car.data.int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_diff = (double)int_diff;
                float_val = 1;
            }
            float_diff -= args->car.data.float_data;
            break;
        default:
            type_error(args->car);
        }
        args = args->cdr;
    }
    if (float_val)
        return (struct val){TYPE_FLOAT, {.float_data = float_diff}};
    else
        return (struct val){TYPE_INT, {.int_data = int_diff}};
}

struct val mul_prim(struct val_list *args) {
    long long int_prod = 1;
    double float_prod;
    int float_val = 0;
    while (args != NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            if (float_val)
                float_prod *= (double)args->car.data.int_data;
            else
                int_prod *= args->car.data.int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_prod = (double)int_prod;
                float_val = 1;
            }
            float_prod *= args->car.data.float_data;
            break;
        default:
            type_error(args->car);
        }
        args = args->cdr;
    }
    if (float_val)
        return (struct val){TYPE_FLOAT, {.float_data = float_prod}};
    else
        return (struct val){TYPE_INT, {.int_data = int_prod}};
}

struct val div_prim(struct val_list *args) {
    double quot;
    if (args == NULL) {
        fprintf(stderr, "Error: no arguments");
        exit(2);
    }
    if (args->cdr == NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            return (struct val){TYPE_FLOAT, {.float_data = 1.0 / (double)args->car.data.int_data}};
        case TYPE_FLOAT:
            return (struct val){TYPE_FLOAT, {.float_data = 1.0 / (double)args->car.data.float_data}};
        default:
            type_error(args->car);
        }
    }
    switch (args->car.type) {
    case TYPE_INT:
        quot = (double)args->car.data.int_data;
        break;
    case TYPE_FLOAT:
        quot = args->car.data.float_data;
        break;
    default:
        type_error(args->car);
    }
    args = args->cdr;
    while (args != NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            quot /= (double)args->car.data.int_data;
            break;
        case TYPE_FLOAT:
            quot /= args->car.data.float_data;
            break;
        default:
            type_error(args->car);
        }
        args = args->cdr;
    }
    return (struct val){TYPE_FLOAT, {.float_data = quot}};
}

struct val false_val = (struct val){TYPE_BOOL, {.int_data = 0}};
struct val true_val = (struct val){TYPE_BOOL, {.int_data = 1}};

struct val equ_prim(struct val_list *args) {
    if (args == NULL)
        return true_val;
    while (args->cdr != NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            switch (args->cdr->car.type) {
            case TYPE_INT:
                if (args->car.data.int_data != args->cdr->car.data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args->car.data.int_data != args->cdr->car.data.float_data)
                    return false_val;
                break;
            default:
                type_error(args->cdr->car);
            }
            break;
        case TYPE_FLOAT:
            switch (args->cdr->car.type) {
            case TYPE_INT:
                if (args->car.data.float_data != args->cdr->car.data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args->car.data.float_data != args->cdr->car.data.float_data)
                    return false_val;
                break;
            default:
                type_error(args->cdr->car);
            }
            break;
        default:
            type_error(args->car);
        }
        args = args->cdr;
    }
    return true_val;
}

int eq(struct val val1, struct val val2) {
    if (val1.type != val2.type)
        return 0;
    switch (val1.type) {
    case TYPE_INT:
    case TYPE_BOOL:
        return val1.data.int_data == val2.data.int_data;
    case TYPE_FLOAT:
        return val1.data.float_data == val2.data.float_data;
    case TYPE_STRING:
    case TYPE_SYMBOL:
        return val1.data.string_data == val2.data.string_data;
    case TYPE_PRIM:
        return val1.data.prim_data == val2.data.prim_data;
    case TYPE_LAMBDA:
        return val1.data.lambda_data == val2.data.lambda_data;
    case TYPE_PAIR:
        return val1.data.pair_data == val2.data.pair_data;
    case TYPE_NIL:
    case TYPE_VOID:
    case TYPE_BROKEN_HEART:
        return 1;
    }
}

struct val eq_prim(struct val_list *args) {
    if (args == NULL || args->cdr == NULL || args->cdr->cdr != NULL)
        args_error();
    return (struct val){TYPE_BOOL, {.int_data = eq(args->car, args->cdr->car)}};
}

struct val pair_prim(struct val_list *args) {
    if (args == NULL || args->cdr != NULL)
        args_error();
    return (struct val){TYPE_BOOL, {.int_data = args->car.type == TYPE_PAIR}};
}

struct val null_prim(struct val_list *args) {
    if (args == NULL || args->cdr != NULL)
        args_error();
    return (struct val){TYPE_BOOL, {.int_data = args->car.type == TYPE_NIL}};
}

struct val cons_prim(struct val_list *args) {
    if (args == NULL || args->cdr == NULL || args->cdr->cdr != NULL)
        args_error();
    struct pair *pair = alloc_pair();
    pair->car = args->car;
    pair->cdr = args->cdr->car;
    return (struct val){TYPE_PAIR, {.pair_data = pair}};
}

struct val car_prim(struct val_list *args) {
    if (args == NULL || args->cdr != NULL)
        args_error();
    if (args->car.type != TYPE_PAIR)
        type_error(args->car);
    return args->car.data.pair_data->car;
}

struct val cdr_prim(struct val_list *args) {
    if (args == NULL || args->cdr != NULL)
        args_error();
    if (args->car.type != TYPE_PAIR)
        type_error(args->car);
    return args->car.data.pair_data->cdr;
}

struct val list_prim(struct val_list *list) {
    if (list == NULL)
        return (struct val){TYPE_NIL};
    struct pair *cdr;
    struct val pair = (struct val){TYPE_PAIR, {.pair_data = alloc_pair()}};
    struct val val = pair;
    gc_push_val(&val);
    gc_push_val(&pair);
    while (list->cdr != NULL) {
        pair.data.pair_data->car = list->car;
        pair.data.pair_data->cdr = (struct val){TYPE_NIL};
        cdr = alloc_pair();
        pair.data.pair_data->cdr = (struct val){TYPE_PAIR, {.pair_data = cdr}};
        pair = pair.data.pair_data->cdr;
        list = list->cdr;
    }
    pair.data.pair_data->car = list->car;
    pair.data.pair_data->cdr = (struct val){TYPE_NIL};
    gc_pop_val();
    gc_pop_val();
    return val;
}

struct val not_prim(struct val_list *args) {
    if (args == NULL || args->cdr != NULL)
        args_error();
    if (is_true(args->car))
        return (struct val){TYPE_BOOL, {.int_data = 0}};
    else
        return (struct val){TYPE_BOOL, {.int_data = 1}};
}

struct val set_car_prim(struct val_list *args) {
    if (args == NULL || args->cdr == NULL || args->cdr->cdr != NULL)
        args_error();
    if (args->car.type != TYPE_PAIR)
        type_error(args->car);
    args->car.data.pair_data->car = args->cdr->car;
    return (struct val){TYPE_VOID};
}

struct val set_cdr_prim(struct val_list *args) {
    if (args == NULL || args->cdr == NULL || args->cdr->cdr != NULL)
        args_error();
    if (args->car.type != TYPE_PAIR)
        type_error(args->car);
    args->car.data.pair_data->cdr = args->cdr->car;
    return (struct val){TYPE_VOID};
}

int equal(struct val val1, struct val val2) {
    switch (val1.type) {
    case TYPE_STRING:
        return val2.type == TYPE_STRING && strcmp(val1.data.string_data, val2.data.string_data) == 0;
    case TYPE_PAIR:
        return val2.type == TYPE_PAIR
            && equal(val1.data.pair_data->car, val2.data.pair_data->car)
            && equal(val1.data.pair_data->cdr, val2.data.pair_data->cdr);
    default:
        return eq(val1, val2);
    }
}

struct val equal_prim(struct val_list *args) {
    if (args == NULL || args->cdr == NULL || args->cdr->cdr != NULL)
        args_error();
    return (struct val){TYPE_BOOL, {.int_data = equal(args->car, args->cdr->car)}};
}

struct val display_prim(struct val_list *args) {
    if (args == NULL || args->cdr != NULL)
        args_error();
    inner_display_val(args->car);
    return (struct val){TYPE_VOID};
}

struct val newline_prim(struct val_list *args) {
    if (args != NULL)
        args_error();
    putchar('\n');
    return (struct val){TYPE_VOID};
}

struct val error_prim(struct val_list *args) {
    if (args == NULL) {
        fprintf("Error.\n");
        exit(2);
    }
    fprintf("Error: ");
    while (args->cdr != NULL) {
        display_val(args->car);
        putchar(' ');
        args = args->cdr;
    }
    display_val(args->car);
    putchar('\n');
    exit(2);
}

struct val (*prim_vals[])(struct val_list *) =
    {add_prim, sub_prim, mul_prim, div_prim,
     equ_prim, eq_prim, pair_prim, null_prim,
     cons_prim, car_prim, cdr_prim, list_prim,
     not_prim, set_car_prim, set_cdr_prim, equal_prim,
     display_prim, newline_prim, error_prim};
char *prim_vars[] =
    {"+", "-", "*", "/",
     "=", "eq?", "pair?", "null?",
     "cons", "car", "cdr", "list",
     "not", "set-car!", "set-cdr!", "equal?",
     "display", "newline", "error"};

size_t prims = sizeof(prim_vals) / sizeof(struct val (*)(struct val_list *));
