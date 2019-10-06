#include <limits.h>
#include <string.h>

#include "bigint.h"

#if PTRDIFF_MAX <= LLONG_MAX
#define abs llabs
#else
#define abs imaxabs
#endif

Bigint *bigint_add(Bigint *m, Bigint *n) {
    size_t m_len = (size_t)abs(m->len);
    size_t n_len = (size_t)abs(n->len);
    if (abs(m->len) > abs(n->len)) { // Ensure |m->len| <= |n->len|
        Bigint *t = m;
        m = n;
        n = t;
        m_len = (size_t)abs(m->len);
        n_len = (size_t)abs(n->len);
    }
    Bigint *r = gc_alloc(sizeof(Bigint) + (n_len + 1) * sizeof(bi_base));
    r->len = n_len;
    bi_base carry = 0;
    for (size_t i = 0; i < m->len; i++) { // Add common part
        bi_double_base x = (bi_double_base)m->digits[i] + n->digits[i] + carry;
        r->digits[i] = x & BI_BASE_MAX;
        carry = x >> BI_BASE_BITS;
    }
    size_t i = m_len;
    if (carry) { // Copy bottom part of n until carry is clear
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
    return r;
}
