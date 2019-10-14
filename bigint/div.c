#include <string.h>

#include "bigint.h"
#include "../types.h"
#include "../exec_stack.h"
#include "../safestd.h"

Bigint *bigint_div(Val xv, Val yv) {
    // Division is performed using Algorithm D from chapter 4.3.1 of volume 2
    // of The Art of Computer Programming.
    size_t x_len = bilabs(xv.bigint_data->len) + 1;
    size_t y_len = bilabs(yv.bigint_data->len);
    if (x_len - 1 < y_len) {
        Bigint *r = gc_alloc_bigint(0);
        r->len = 0;
        return r;
    }
    size_t r_len = x_len - y_len;
    stack_push(xv);
    stack_push(yv);
    Bigint *r = gc_alloc_bigint(r_len);
    Bigint *y0 = stack_pop().bigint_data;
    Bigint *x0 = stack_pop().bigint_data;
    // Normalize x and y so that highest bit of y is set
    bi_base *y = s_malloc(y_len * sizeof(bi_base));
    bi_base *x = s_malloc(x_len * sizeof(bi_base));
    int shift_amount = clz(y0->digits[y_len - 1]);
    if (shift_amount > 0) {
        y[y_len - 1] = y0->digits[y_len - 1] << shift_amount;
        for (size_t i = y_len - 1; i-- > 0; ) {
            y[i + 1] |= y0->digits[i] >> (BI_BASE_BITS - shift_amount);
            y[i] = y0->digits[i] << shift_amount;
        }
        x[x_len - 1] = 0;
        for (size_t i = x_len - 1; i-- > 0; ) {
            x[i + 1] |= x0->digits[i] >> (BI_BASE_BITS - shift_amount);
            x[i] = x0->digits[i] << shift_amount;
        }
    } else {
        memcpy(y, y0->digits, y_len * sizeof(bi_base));
        memcpy(x, x0->digits, x_len * sizeof(bi_base));
        x[x_len - 1] = 0;
    }
    // Repeatedly divide (y_len + 1) highest digits of x by y
    for (size_t i = r_len; i-- > 0; ) {
        // Calculate estimate of quotient
        bi_double_base x_top = (bi_double_base)x[i + y_len] << BI_BASE_BITS | x[i + y_len - 1];
        bi_double_base q = x_top / y[y_len - 1];
        bi_double_base rem = x_top % y[y_len - 1];
        // Reduce error of estimate
//      Uncomment when division by one digit is separated
//      if (q > BI_BASE_MAX || q * y[y_len - 2] > ((rem << BI_BASE_BITS) | x[i + y_len - 2])) {
        if (q > BI_BASE_MAX || (y_len > 1 && q * y[y_len - 2] > ((rem << BI_BASE_BITS) | x[i + y_len - 2]))) {
            q--;
            rem += y[y_len - 1];
//          if (rem <= BI_BASE_MAX && (q > BI_BASE_MAX || q * y[y_len - 2] > ((rem << BI_BASE_BITS) | x[i + y_len - 2]))) {
            if (rem <= BI_BASE_MAX && (q > BI_BASE_MAX || (y_len > 1 && q * y[y_len - 2] > ((rem << BI_BASE_BITS) | x[i + y_len - 2])))) {
                q--;
                rem += y[y_len - 1];
            }
        }
        // Subtract q*y from x
        bi_base sub_carry = 0;
        bi_base mul_carry = 0;
        for (size_t j = 0; j < y_len; j++) {
            bi_double_base mul_d = q * y[j] + mul_carry;
            mul_carry = mul_d >> BI_BASE_BITS;
            bi_double_base sub_d = (bi_double_base)x[i + j] - (bi_base)mul_d - sub_carry;
            x[i + j] = (bi_base)sub_d;
            sub_carry = (sub_d >> BI_BASE_BITS) & 1;
        }
        bi_double_base sub_d = x[i + y_len] - mul_carry - sub_carry;
        x[i + y_len] = (bi_base)sub_d;
        sub_carry = (sub_d >> BI_BASE_BITS) & 1;
        // Add back if q was an overestimate
        if (sub_carry) {
            bi_base carry = 0;
            for (size_t j = 0; j < y_len; j++) {
                bi_double_base d = (bi_double_base)x[i + j] + y[j] + carry;
                x[i + j] = (bi_base)d;
                carry = d >> BI_BASE_BITS;
            }
            x[i + y_len] += carry;
            q--;
        }
        r->digits[i] = (bi_base)q;
    }
    free(x);
    free(y);
    if (r->digits[r_len - 1] == 0)
        r_len--;
    r->len = (ptrdiff_t)r_len * ((x0->len >= 0) ? 1 : -1) * ((y0->len >= 0) ? 1 : -1);
    return r;
}
