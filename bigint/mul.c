#include <string.h>

#include "bigint.h"
#include "../types.h"
#include "../exec_stack.h"

// TODO handle multiplication by zero separately
Bigint *bigint_mul(Val xv, Val yv) {
    size_t r_len = bilabs(xv.bigint_data->len) + bilabs(yv.bigint_data->len);
    stack_push(xv);
    stack_push(yv);
    Bigint *r = gc_alloc_bigint(r_len + 1);
    memset(r->digits, 0, r_len * sizeof(bi_base));
    Bigint *n = stack_pop().bigint_data;
    Bigint *m = stack_pop().bigint_data;
    size_t n_len = bilabs(n->len);
    size_t m_len = bilabs(m->len);
    for (size_t i = 0; i < m_len; i++) {
        bi_base carry = 0;
        for (size_t j = 0; j < n_len; j++) {
            bi_double_base d = (bi_double_base)n->digits[j] * m->digits[i] + r->digits[i + j] + carry;
            r->digits[i + j] = (bi_base)d;
            carry = d >> BI_BASE_BITS;
        }
        r->digits[i + n_len] = carry;
    }
    r->len = (ptrdiff_t)r_len * ((n->len >= 0) ? 1 : -1) * ((m->len >= 0) ? 1 : -1);
    return r;
}
