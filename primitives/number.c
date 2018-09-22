#include <stdio.h>
#include <stdlib.h>

#include "../expr.h"
#include "assert.h"

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

struct val lt_prim(struct val_list *args) {
    if (args == NULL)
        return true_val;
    while (args->cdr != NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            switch (args->cdr->car.type) {
            case TYPE_INT:
                if (args->car.data.int_data >= args->cdr->car.data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args->car.data.int_data >= args->cdr->car.data.float_data)
                    return false_val;
                break;
            default:
                type_error(args->cdr->car);
            }
            break;
        case TYPE_FLOAT:
            switch (args->cdr->car.type) {
            case TYPE_INT:
                if (args->car.data.float_data >= args->cdr->car.data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args->car.data.float_data >= args->cdr->car.data.float_data)
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

struct val gt_prim(struct val_list *args) {
    if (args == NULL)
        return true_val;
    while (args->cdr != NULL) {
        switch (args->car.type) {
        case TYPE_INT:
            switch (args->cdr->car.type) {
            case TYPE_INT:
                if (args->car.data.int_data <= args->cdr->car.data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args->car.data.int_data <= args->cdr->car.data.float_data)
                    return false_val;
                break;
            default:
                type_error(args->cdr->car);
            }
            break;
        case TYPE_FLOAT:
            switch (args->cdr->car.type) {
            case TYPE_INT:
                if (args->car.data.float_data <= args->cdr->car.data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args->car.data.float_data <= args->cdr->car.data.float_data)
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
