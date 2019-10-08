#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "bigint.h"

#if PTRDIFF_MAX <= LLONG_MAX
#define abs llabs
#else
#define abs imaxabs
#endif

// TODO avoid GC of m and n
// TODO put correct sign on result
static Bigint *bigint_add_sub(Bigint *m, Bigint *n, int subtract) {
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
    if (((m->len < 0) != (n->len < 0)) == subtract) { // Same signs
        r->len = n->len;
        bi_base carry = 0;
        for (size_t i = 0; i < m->len; i++) { // Add common part
            bi_double_base x = (bi_double_base)m->digits[i] + n->digits[i] + carry;
            r->digits[i] = x;
            carry = x >> BI_BASE_BITS;
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
    } else { // Different signs
        if (m_len == n_len) {
            bi_base carry = 0;
            while (n_len-- > 0 && m->digits[n_len] == n->digits[n_len]) // Remove common leading digits
                ;
            n_len++;
            m_len = n_len;
            if (n_len > 0 && m->digits[n_len - 1] > n->digits[n_len - 1]) { // Ensure |m| <= |n|
                Bigint *t = m;
                m = n;
                n = t;
            }
        }
        bi_base carry = 0;
        for (size_t i = 0; i < m_len; i++) { // Subtract common part
            bi_double_base x = (bi_double_base)n->digits[i] - m->digits[i] - carry;
            r->digits[i] = x;
            carry = (x >> BI_BASE_BITS) & 1;
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
        r->len = n_len;
    }
    return r;
}

Bigint *bigint_add(Bigint *m, Bigint *n) {
    return bigint_add_sub(m, n, 0);
}

Bigint *bigint_sub(Bigint *m, Bigint *n) {
    return bigint_add_sub(m, n, 1);
}
