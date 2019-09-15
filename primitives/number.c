#include <stdio.h>
#include <stdlib.h>

#include "number.h"
#include "../types.h"
#include "assert.h"

Val add_prim(Val *args, uint32_t num) {
    long long int_sum = 0;
    double float_sum;
    int float_val = 0;
    for (Val *arg_ptr = args; arg_ptr < args + num; arg_ptr++) {
        switch (arg_ptr->type) {
        case TYPE_INT:
            if (float_val)
                float_sum += (double)arg_ptr->int_data;
            else
                int_sum += arg_ptr->int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_sum = (double)int_sum;
                float_val = 1;
            }
            float_sum += arg_ptr->float_data;
            break;
        default:
            type_error(*arg_ptr);
        }
    }
    if (float_val)
        return (Val){TYPE_FLOAT, {.float_data = float_sum}};
    else
        return (Val){TYPE_INT, {.int_data = int_sum}};
}

Val sub_prim(Val *args, uint32_t num) {
    long long int_diff = 0;
    double float_diff = 0;
    int float_val = 0;
    args_assert(num != 0);
    if (num == 1) {
        switch (args->type) {
        case TYPE_INT:
            return (Val){TYPE_INT, {.int_data = - args->int_data}};
        case TYPE_FLOAT:
            return (Val){TYPE_FLOAT, {.float_data = - args->float_data}};
        default:
            type_error(*args);
        }
    }
    switch (args->type) {
    case TYPE_INT:
        int_diff = args->int_data;
        break;
    case TYPE_FLOAT:
        float_diff = args->float_data;
        float_val = 1;
        break;
    default:
        type_error(*args);
    }
    for (Val *arg_ptr = args + 1; arg_ptr < args + num; arg_ptr++) {
        switch (arg_ptr->type) {
        case TYPE_INT:
            if (float_val)
                float_diff -= (double)arg_ptr->int_data;
            else
                int_diff -= arg_ptr->int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_diff = (double)int_diff;
                float_val = 1;
            }
            float_diff -= arg_ptr->float_data;
            break;
        default:
            type_error(*arg_ptr);
        }
    }
    if (float_val)
        return (Val){TYPE_FLOAT, {.float_data = float_diff}};
    else
        return (Val){TYPE_INT, {.int_data = int_diff}};
}

Val mul_prim(Val *args, uint32_t num) {
    long long int_prod = 1;
    double float_prod;
    int float_val = 0;
    for (Val *arg_ptr = args; arg_ptr < args + num; arg_ptr++) {
        switch (arg_ptr->type) {
        case TYPE_INT:
            if (float_val)
                float_prod *= (double)arg_ptr->int_data;
            else
                int_prod *= arg_ptr->int_data;
            break;
        case TYPE_FLOAT:
            if (!float_val) {
                float_prod = (double)int_prod;
                float_val = 1;
            }
            float_prod *= arg_ptr->float_data;
            break;
        default:
            type_error(*arg_ptr);
        }
    }
    if (float_val)
        return (Val){TYPE_FLOAT, {.float_data = float_prod}};
    else
        return (Val){TYPE_INT, {.int_data = int_prod}};
}

Val div_prim(Val *args, uint32_t num) {
    double quot = 0;
    args_assert(num != 0);
    if (num == 1) {
        switch (args->type) {
        case TYPE_INT:
            return (Val){TYPE_FLOAT, {.float_data = 1.0 / (double)args->int_data}};
        case TYPE_FLOAT:
            return (Val){TYPE_FLOAT, {.float_data = 1.0 / (double)args->float_data}};
        default:
            type_error(*args);
        }
    }
    switch (args->type) {
    case TYPE_INT:
        quot = (double)args->int_data;
        break;
    case TYPE_FLOAT:
        quot = args->float_data;
        break;
    default:
        type_error(*args);
    }
    for (Val *arg_ptr = args + 1; arg_ptr < args + num; arg_ptr++) {
        switch (arg_ptr->type) {
        case TYPE_INT:
            quot /= (double)arg_ptr->int_data;
            break;
        case TYPE_FLOAT:
            quot /= arg_ptr->float_data;
            break;
        default:
            type_error(*arg_ptr);
        }
    }
    return (Val){TYPE_FLOAT, {.float_data = quot}};
}

Val equ_prim(Val *args, uint32_t num) {
    if (num == 0)
        return true_val;
    for (Val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        switch (arg_ptr[0].type) {
        case TYPE_INT:
            switch (arg_ptr[1].type) {
            case TYPE_INT:
                if (arg_ptr[0].int_data != arg_ptr[1].int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (arg_ptr[0].int_data != arg_ptr[1].float_data)
                    return false_val;
                break;
            default:
                type_error(arg_ptr[1]);
            }
            break;
        case TYPE_FLOAT:
            switch (arg_ptr[1].type) {
            case TYPE_INT:
                if (arg_ptr[0].float_data != arg_ptr[1].int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (arg_ptr[0].float_data != arg_ptr[1].float_data)
                    return false_val;
                break;
            default:
                type_error(arg_ptr[1]);
            }
            break;
        default:
            type_error(arg_ptr[0]);
        }
    }
    return true_val;
}

Val lt_prim(Val *args, uint32_t num) {
    if (num == 0)
        return true_val;
    for (Val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        switch (arg_ptr[0].type) {
        case TYPE_INT:
            switch (arg_ptr[1].type) {
            case TYPE_INT:
                if (arg_ptr[0].int_data >= arg_ptr[1].int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (arg_ptr[0].int_data >= arg_ptr[1].float_data)
                    return false_val;
                break;
            default:
                type_error(arg_ptr[1]);
            }
            break;
        case TYPE_FLOAT:
            switch (arg_ptr[1].type) {
            case TYPE_INT:
                if (arg_ptr[0].float_data >= arg_ptr[1].int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (arg_ptr[0].float_data >= arg_ptr[1].float_data)
                    return false_val;
                break;
            default:
                type_error(arg_ptr[1]);
            }
            break;
        default:
            type_error(arg_ptr[0]);
        }
    }
    return true_val;
}

Val gt_prim(Val *args, uint32_t num) {
    if (num == 0)
        return true_val;
    for (Val *arg_ptr = args; arg_ptr < args + num - 1; arg_ptr++) {
        switch (arg_ptr[0].type) {
        case TYPE_INT:
            switch (arg_ptr[1].type) {
            case TYPE_INT:
                if (arg_ptr[0].int_data <= arg_ptr[1].int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (arg_ptr[0].int_data <= arg_ptr[1].float_data)
                    return false_val;
                break;
            default:
                type_error(arg_ptr[1]);
            }
            break;
        case TYPE_FLOAT:
            switch (arg_ptr[1].type) {
            case TYPE_INT:
                if (arg_ptr[0].float_data <= arg_ptr[1].int_data)
                    return false_val;
                break;
            case TYPE_FLOAT:
                if (arg_ptr[0].float_data <= arg_ptr[1].float_data)
                    return false_val;
                break;
            default:
                type_error(arg_ptr[1]);
            }
            break;
        default:
            type_error(arg_ptr[0]);
        }
    }
    return true_val;
}
