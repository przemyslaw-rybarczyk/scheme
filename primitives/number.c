#include <stdio.h>
#include <stdlib.h>

#include "number.h"
#include "../expr.h"
#include "assert.h"

struct val add_prim(struct val *args, int num) {
    long long int_sum = 0;
    double float_sum;
    int float_val = 0;
    for (struct val *arg_ptr = args; arg_ptr < args + num; arg_ptr++) {
        switch (arg_ptr->type) {
        case TYPE_INT:
            if (float_val)
                float_sum += (double)arg_ptr->data.int_data;
            else
                int_sum += arg_ptr->data.int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_sum = (double)int_sum;
                float_val = 1;
            }
            float_sum += arg_ptr->data.float_data;
            break;
        default:
            type_error(*arg_ptr);
        }
    }
    if (float_val)
        return (struct val){TYPE_FLOAT, {.float_data = float_sum}};
    else
        return (struct val){TYPE_INT, {.int_data = int_sum}};
}

struct val sub_prim(struct val *args, int num) {
    long long int_diff;
    double float_diff;
    int float_val = 0;
    args_assert(num != 0);
    if (num == 1) {
        switch (args->type) {
        case TYPE_INT:
            return (struct val){TYPE_INT, {.int_data = - args->data.int_data}};
        case TYPE_FLOAT:
            return (struct val){TYPE_FLOAT, {.float_data = - args->data.float_data}};
        default:
            type_error(*args);
        }
    }
    switch (args->type) {
    case TYPE_INT:
        int_diff = args->data.int_data;
        break;
    case TYPE_FLOAT:
        float_diff = args->data.float_data;
        float_val = 1;
        break;
    default:
        type_error(*args);
    }
    for (struct val *arg_ptr = args + 1; arg_ptr < args + num; arg_ptr++) {
        switch (arg_ptr->type) {
        case TYPE_INT:
            if (float_val)
                float_diff -= (double)arg_ptr->data.int_data;
            else
                int_diff -= arg_ptr->data.int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_diff = (double)int_diff;
                float_val = 1;
            }
            float_diff -= arg_ptr->data.float_data;
            break;
        default:
            type_error(*arg_ptr);
        }
    }
    if (float_val)
        return (struct val){TYPE_FLOAT, {.float_data = float_diff}};
    else
        return (struct val){TYPE_INT, {.int_data = int_diff}};
}

struct val mul_prim(struct val *args, int num) {
    long long int_prod = 1;
    double float_prod;
    int float_val = 0;
    for (struct val *arg_ptr = args; arg_ptr < args + num; arg_ptr++) {
        switch (arg_ptr->type) {
        case TYPE_INT:
            if (float_val)
                float_prod *= (double)arg_ptr->data.int_data;
            else
                int_prod *= arg_ptr->data.int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_prod = (double)int_prod;
                float_val = 1;
            }
            float_prod *= arg_ptr->data.float_data;
            break;
        default:
            type_error(*arg_ptr);
        }
    }
    if (float_val)
        return (struct val){TYPE_FLOAT, {.float_data = float_prod}};
    else
        return (struct val){TYPE_INT, {.int_data = int_prod}};
}

struct val div_prim(struct val *args, int num) {
    double quot;
    args_assert(num != 0);
    if (num == 1) {
        switch (args->type) {
        case TYPE_INT:
            return (struct val){TYPE_FLOAT, {.float_data = 1.0 / (double)args->data.int_data}};
        case TYPE_FLOAT:
            return (struct val){TYPE_FLOAT, {.float_data = 1.0 / (double)args->data.float_data}};
        default:
            type_error(*args);
        }
    }
    switch (args->type) {
    case TYPE_INT:
        quot = (double)args->data.int_data;
        break;
    case TYPE_FLOAT:
        quot = args->data.float_data;
        break;
    default:
        type_error(*args);
    }
    for (struct val *arg_ptr = args + 1; arg_ptr < args + num; arg_ptr++) {
        switch (arg_ptr->type) {
        case TYPE_INT:
            quot /= (double)arg_ptr->data.int_data;
            break;
        case TYPE_FLOAT:
            quot /= arg_ptr->data.float_data;
            break;
        default:
            type_error(*arg_ptr);
        }
    }
    return (struct val){TYPE_FLOAT, {.float_data = quot}};
}

struct val false_val = (struct val){TYPE_BOOL, {.int_data = 0}};
struct val true_val = (struct val){TYPE_BOOL, {.int_data = 1}};

struct val equ_prim(struct val *args, int num) {
    if (num == 0)
        return true_val;
    for (struct val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        switch (args[0].type) {
        case TYPE_INT:
            switch (args[1].type) {
            case TYPE_INT:
                if (args[0].data.int_data != args[1].data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args[0].data.int_data != args[1].data.float_data)
                    return false_val;
                break;
            default:
                type_error(args[1]);
            }
            break;
        case TYPE_FLOAT:
            switch (args[1].type) {
            case TYPE_INT:
                if (args[0].data.float_data != args[1].data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args[0].data.float_data != args[1].data.float_data)
                    return false_val;
                break;
            default:
                type_error(args[1]);
            }
            break;
        default:
            type_error(args[0]);
        }
    }
    return true_val;
}

struct val lt_prim(struct val *args, int num) {
    if (num == 0)
        return true_val;
    for (struct val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        switch (args[0].type) {
        case TYPE_INT:
            switch (args[1].type) {
            case TYPE_INT:
                if (args[0].data.int_data >= args[1].data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args[0].data.int_data >= args[1].data.float_data)
                    return false_val;
                break;
            default:
                type_error(args[1]);
            }
            break;
        case TYPE_FLOAT:
            switch (args[1].type) {
            case TYPE_INT:
                if (args[0].data.float_data >= args[1].data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args[0].data.float_data >= args[1].data.float_data)
                    return false_val;
                break;
            default:
                type_error(args[1]);
            }
            break;
        default:
            type_error(args[0]);
        }
    }
    return true_val;
}

struct val gt_prim(struct val *args, int num) {
    if (num == 0)
        return true_val;
    for (struct val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        switch (args[0].type) {
        case TYPE_INT:
            switch (args[1].type) {
            case TYPE_INT:
                if (args[0].data.int_data <= args[1].data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args[0].data.int_data <= args[1].data.float_data)
                    return false_val;
                break;
            default:
                type_error(args[1]);
            }
            break;
        case TYPE_FLOAT:
            switch (args[1].type) {
            case TYPE_INT:
                if (args[0].data.float_data <= args[1].data.int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (args[0].data.float_data <= args[1].data.float_data)
                    return false_val;
                break;
            default:
                type_error(args[1]);
            }
            break;
        default:
            type_error(args[0]);
        }
    }
    return true_val;
}
