#include <string.h>

#include "bigint.h"
#include "ops.h"

Bigint *bigint_mul(Bigint *m, Bigint *n, Bigint *r) {
    if (m->len == 0 || n->len == 0) {
        r->len = 0;
        return r;
    }
    size_t r_len = bilabs(m->len) + bilabs(n->len);
    memset(r->digits, 0, r_len * sizeof(bi_base));
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
    if (r->digits[r_len - 1] == 0)
        r_len--;
    r->len = (ptrdiff_t)r_len * ((n->len >= 0) ? 1 : -1) * ((m->len >= 0) ? 1 : -1);
    return r;
}
