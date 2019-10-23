#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "number.h"
#include "../types.h"
#include "../bigint/ops.h"
#include "../exec_stack.h"
#include "../memory.h"
#include "../safestd.h"
#include "assert.h"

Val add_prim(Val *args, uint32_t num) {
    if ((args[0].type == TYPE_BIGINT || args[0].type == TYPE_CONST_BIGINT) && (args[1].type == TYPE_BIGINT || args[1].type == TYPE_CONST_BIGINT)) {
        Bigint *r = gc_alloc_bigint(BIGINT_ADD_LEN(args[0].bigint_data, args[1].bigint_data));
        return (Val){TYPE_BIGINT, {.bigint_data = bigint_add(args[0].bigint_data, args[1].bigint_data, r)}};
    }
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
    if ((args[0].type == TYPE_BIGINT || args[0].type == TYPE_CONST_BIGINT) && (args[1].type == TYPE_BIGINT || args[1].type == TYPE_CONST_BIGINT)) {
        Bigint *r = gc_alloc_bigint(BIGINT_SUB_LEN(args[0].bigint_data, args[1].bigint_data));
        return (Val){TYPE_BIGINT, {.bigint_data = bigint_sub(args[0].bigint_data, args[1].bigint_data, r)}};
    }
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
    if ((args[0].type == TYPE_BIGINT || args[0].type == TYPE_CONST_BIGINT) && (args[1].type == TYPE_BIGINT || args[1].type == TYPE_CONST_BIGINT)) {
        Bigint *r = gc_alloc_bigint(BIGINT_MUL_LEN(args[0].bigint_data, args[1].bigint_data));
        return (Val){TYPE_BIGINT, {.bigint_data = bigint_mul(args[0].bigint_data, args[1].bigint_data, r)}};
    }
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
    if ((args[0].type == TYPE_BIGINT || args[0].type == TYPE_CONST_BIGINT) && (args[1].type == TYPE_BIGINT || args[1].type == TYPE_CONST_BIGINT)) {
        Fraction *r = gc_alloc(sizeof(Fraction));
        Bigint *d = s_malloc(sizeof(Bigint) + BIGINT_GCD_LEN(args[0].bigint_data, args[1].bigint_data) * sizeof(bi_base));
        bigint_gcd(args[0].bigint_data, args[1].bigint_data, d);
        r->numerator = gc_alloc_bigint(BIGINT_DIV_LEN(args[0].bigint_data, d));
        r->denominator = gc_alloc_bigint(BIGINT_DIV_LEN(args[1].bigint_data, d));
        bigint_div(args[0].bigint_data, d, r->numerator);
        bigint_div(args[1].bigint_data, d, r->denominator);
        free(d);
        return (Val){TYPE_FRACTION, {.fraction_data = r}};
    }
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

Val quotient_prim(Val *args, uint32_t num) {
    if ((args[0].type == TYPE_BIGINT || args[0].type == TYPE_CONST_BIGINT) && (args[1].type == TYPE_BIGINT || args[1].type == TYPE_CONST_BIGINT)) {
        Bigint *r = gc_alloc_bigint(BIGINT_DIV_LEN(args[0].bigint_data, args[1].bigint_data));
        return (Val){TYPE_BIGINT, {.bigint_data = bigint_div(args[0].bigint_data, args[1].bigint_data, r)}};
    }
    return (Val){TYPE_VOID};
}

Val mod_prim(Val *args, uint32_t num) {
    if ((args[0].type == TYPE_BIGINT || args[0].type == TYPE_CONST_BIGINT) && (args[1].type == TYPE_BIGINT || args[1].type == TYPE_CONST_BIGINT)) {
        Bigint *r = gc_alloc_bigint(BIGINT_MOD_LEN(args[0].bigint_data, args[1].bigint_data));
        return (Val){TYPE_BIGINT, {.bigint_data = bigint_mod(args[0].bigint_data, args[1].bigint_data, r)}};
    }
    return (Val){TYPE_VOID};
}

Val gcd_prim(Val *args, uint32_t num) {
    if ((args[0].type == TYPE_BIGINT || args[0].type == TYPE_CONST_BIGINT) && (args[1].type == TYPE_BIGINT || args[1].type == TYPE_CONST_BIGINT)) {
        Bigint *r = gc_alloc_bigint(BIGINT_GCD_LEN(args[0].bigint_data, args[1].bigint_data));
        return (Val){TYPE_BIGINT, {.bigint_data = bigint_gcd(args[0].bigint_data, args[1].bigint_data, r)}};
    }
    return (Val){TYPE_VOID};
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

Val make_rectangular_prim(Val *args, uint32_t num) {
    args_assert(num == 2);
    if (args[0].type == TYPE_FLOAT && args[1].type == TYPE_FLOAT) {
        double complex *z = gc_alloc(sizeof(double complex));
        *z = args[0].float_data + args[1].float_data * I;
        return (Val){TYPE_FLOAT_COMPLEX, {.float_complex_data = z}};
    }
    Complex *z = gc_alloc(sizeof(Complex));
    z->real = args[0];
    z->imag = args[1];
    return (Val){TYPE_COMPLEX, {.complex_data = z}};
}
