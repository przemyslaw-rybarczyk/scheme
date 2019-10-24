#include <string.h>

#include "bigint.h"
#include "ops.h"
#include "../safestd.h"

void bigint_abs(Bigint *x, Bigint *r) {
    memcpy(r->digits, x->digits, bilabs(x->len) * sizeof(bi_base));
    r->len = bilabs(x->len);
}

typedef struct Shift_count {
    size_t digits;
    int bits;
} Shift_count;

static Shift_count bigint_ctz(Bigint *x) {
    for (size_t i = 0; ; i++) {
        if (x->digits[i] != 0)
            return (Shift_count){i, ctz(x->digits[i])};
    }
}

static void shift_right(Bigint *x, Shift_count sc, Bigint *r) {
    size_t len = bilabs(x->len);
    if (sc.bits > 0) {
        r->digits[0] = x->digits[sc.digits] >> sc.bits;
        for (size_t i = 1; i < len - sc.digits; i++) {
            r->digits[i - 1] |= x->digits[i + sc.digits] << (BI_BASE_BITS - sc.bits);
            r->digits[i] = x->digits[i + sc.digits] >> sc.bits;
        }
        r->len = (ptrdiff_t)(len - sc.digits);
        if (r->digits[r->len - 1] == 0)
            r->len--;
    } else {
        memcpy(r->digits, x->digits + sc.digits, (len - sc.digits) * sizeof(bi_base));
        r->len = (ptrdiff_t)(len - sc.digits);
    }
}

static void shift_left(Bigint *x, Shift_count sc, Bigint *r) {
    size_t len = bilabs(x->len);
    memset(r->digits, 0, sc.digits * sizeof(bi_base));
    if (sc.bits > 0) {
        r->digits[len - 1 + sc.digits] = x->digits[len - 1] << sc.bits;
        for (size_t i = len - 1; i-- > 0; ) {
            r->digits[i + sc.digits + 1] |= x->digits[i] >> (BI_BASE_BITS - sc.bits);
            r->digits[i + sc.digits] = x->digits[i] << sc.bits;
        }
        r->len = (ptrdiff_t)(len + sc.digits);
        if (r->digits[r->len - 1] == 0)
            r->len--;
    } else {
        memcpy(r->digits + sc.digits, x->digits, len * sizeof(bi_base));
        r->len = (ptrdiff_t)(len + sc.digits);
    }
}

void bigint_gcd(Bigint *x, Bigint *y, Bigint *r) {
    if (x->len == 0) {
        bigint_abs(y, r);
        return;
    }
    if (y->len == 0) {
        bigint_abs(x, r);
        return;
    }
    Bigint *t = s_malloc(sizeof(Bigint) + BIGINT_SUB_LEN(x, y) * sizeof(bi_base));
    Bigint *u = s_malloc(sizeof(Bigint) + BIGINT_SUB_LEN(x, y) * sizeof(bi_base));
    Bigint *v = s_malloc(sizeof(Bigint) + BIGINT_SUB_LEN(x, y) * sizeof(bi_base));
    Shift_count x_sc = bigint_ctz(x);
    Shift_count y_sc = bigint_ctz(y);
    Shift_count r_sc = ((x_sc.digits == y_sc.digits) ? (x_sc.bits < y_sc.bits) : (x_sc.digits < y_sc.digits)) ? x_sc : y_sc;
    shift_right(x, x_sc, u);
    shift_right(y, y_sc, v);
    bigint_sub(u, v, t);
    while (t->len != 0) {
        shift_right(t, bigint_ctz(t), t->len > 0 ? u : v);
        bigint_sub(u, v, t);
    }
    shift_left(u, r_sc, r);
    free(t);
    free(u);
    free(v);
}
