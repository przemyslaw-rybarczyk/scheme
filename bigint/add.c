#include <stdlib.h>
#include <string.h>

#include "bigint.h"
#include "../types.h"
#include "../exec_stack.h"

/* -- bigint_add_sub
 * Adds or subtracts two bigints. The value `subtract` is set to perform
 * subtraction instead of addition. The values x and y may only be of type
 * TYPE_BIGINT or TYPE_CONST_BIGINT and are not checked.
 */
static Bigint *bigint_add_sub(Val xv, Val yv, int subtract) {
    size_t r_len = bilabs(xv.bigint_data->len) > bilabs(yv.bigint_data->len) ? bilabs(xv.bigint_data->len) : bilabs(yv.bigint_data->len);
    stack_push(xv);
    stack_push(yv);
    Bigint *r = gc_alloc_bigint(r_len + 1);
    Bigint *y = stack_pop().bigint_data;
    Bigint *x = stack_pop().bigint_data;
    Bigint *n;
    Bigint *m;
    int sub_sign;
    if (bilabs(x->len) > bilabs(y->len)) { // Ensure |n->len| >= |m->len|
        n = x;
        m = y;
        sub_sign = (x->len >= 0) ? 1 : -1;
    } else {
        n = y;
        m = x;
        sub_sign = ((subtract ? -1 : 1) * y->len >= 0) ? 1 : -1;
    }
    size_t m_len = bilabs(m->len);
    size_t n_len = bilabs(n->len);
    if ((x->len < 0) == ((subtract ? -1 : 1) * y->len < 0)) { // Same signs
        r->len = (ptrdiff_t)n_len;
        bi_base carry = 0;
        for (size_t i = 0; i < m_len; i++) { // Add common part
            bi_double_base d = (bi_double_base)m->digits[i] + n->digits[i] + carry;
            r->digits[i] = (bi_base)d;
            carry = d >> BI_BASE_BITS;
        }
        size_t i = m_len;
        if (carry) { // Add to bottom part of n until carry is clear
            for (; i < n_len && n->digits[i] == BI_BASE_MAX; i++)
                r->digits[i] = 0;
            if (i < n_len) {
                r->digits[i] = n->digits[i] + 1;
                i++;
            } else {
                r->digits[i] = 1;
                r->len++;
            }
        }
        memcpy(r->digits + i, n->digits + i, (n_len - i) * sizeof(bi_base)); // Copy remaining digits
        r->len *= (x->len >= 0) ? 1 : -1;
    } else { // Different signs
        if (m_len == n_len) {
            while (n_len-- > 0 && m->digits[n_len] == n->digits[n_len]) // Remove common leading digits
                ;
            n_len++;
            m_len = n_len;
            if (n_len > 0 && m->digits[n_len - 1] > n->digits[n_len - 1]) { // Ensure |m| <= |n|
                n = x;
                m = y;
                sub_sign *= -1;
            }
        }
        bi_base carry = 0;
        for (size_t i = 0; i < m_len; i++) { // Subtract common part
            bi_double_base d = (bi_double_base)n->digits[i] - m->digits[i] - carry;
            r->digits[i] = (bi_base)d;
            carry = (d >> BI_BASE_BITS) & 1;
        }
        size_t i = m_len;
        if (carry) { // Subtract from part of n until carry is clear
            for (; i < n_len && n->digits[i] == 0; i++)
                r->digits[i] = BI_BASE_MAX;
            r->digits[i] = n->digits[i] - 1;
            i++;
        }
        memcpy(r->digits + i, n->digits + i, (n_len - i) * sizeof(bi_base)); // Copy remaining digits
        while (n_len-- > 0 && r->digits[n_len] == 0) // Remove leading zeroes
            ;
        n_len++;
        r->len = (ptrdiff_t)n_len * sub_sign;
    }
    return r;
}

Bigint *bigint_add(Val x, Val y) {
    return bigint_add_sub(x, y, 0);
}

Bigint *bigint_sub(Val x, Val y) {
    return bigint_add_sub(x, y, 1);
}
