#include <stdlib.h>
#include <string.h>
#include <uchar.h>

#include "string.h"
#include "../safestd.h"
#include "bigint.h"
#include "ops.h"

bi_base read_digit(char32_t c) {
    if (c >= 'a')
        return c - 'a' + 10;
    if (c >= 'A')
        return c - 'A' + 10;
    return c - '0';
}

char32_t write_digit(bi_base i) {
    if (i >= 10)
        return (char32_t)(i - 10 + 'a');
    return (char32_t)(i + '0');
}

#define HEX_CPD (sizeof(bi_base) * 2)

Bigint *read_bigint_hexadecimal(size_t len, char32_t *chars) {
    while (len >= 0 && *chars == '0') {
        chars++;
        len--;
    }
    size_t bi_len = (len + HEX_CPD - 1) / HEX_CPD;
    Bigint *bi = s_malloc(sizeof(Bigint) + bi_len * sizeof(bi_base));
    bi->len = (ptrdiff_t)bi_len;
    memset(bi->digits, 0, bi_len * sizeof(bi_base));
    for (size_t i = 0; i < len; i++) {
        bi_base d = read_digit(chars[len - 1 - i]);
        if (d > 15) {
            free(bi);
            return NULL;
        }
        bi->digits[i / HEX_CPD] |= d << (4 * (i % HEX_CPD));
    }
    return bi;
}

static size_t bigint_sprint_hexdecimal(Bigint *bi, char32_t *chars) {
    size_t k = 0;
    size_t len;
    if (bi->len < 0) {
        chars[k++] = '-';
        len = (size_t)(-bi->len);
    } else {
        len = (size_t)bi->len;
    }
    for (size_t i = len; i-- > 0; ) {
        for (int j = HEX_CPD; j-- > 0; )
            chars[k++] = write_digit((bi->digits[i] >> (4 * j)) & 0xF);
    }
    return k;
}

size_t bigint_sprint_base(Bigint *bi, bi_base base, char32_t *chars) {
    if (bi->len == 0) {
        chars[0] = '0';
        return 1;
    }
    // TODO special versions for other power-of-two bases
    if (base == 16)
        bigint_sprint_hexdecimal(bi, chars);
    Bigint *y = s_malloc(sizeof(Bigint) + sizeof(bi_base));
    y->len = 1;
    y->digits[0] = base;
    Bigint *d = s_malloc(sizeof(Bigint) + BIGINT_MOD_LEN(bi, y) * sizeof(bi_base));
    Bigint *x = s_malloc(sizeof(Bigint) + BIGINT_DIV_LEN(bi, y) * sizeof(bi_base));
    Bigint *q = s_malloc(sizeof(Bigint) + BIGINT_DIV_LEN(bi, y) * sizeof(bi_base));
    size_t i = 0;
    bigint_mod(bi, y, d);
    chars[i++] = write_digit(d->len ? d->digits[0] : 0);
    bigint_div(bi, y, x);
    while (x->len != 0) {
        bigint_mod(x, y, d);
        chars[i++] = write_digit(d->len ? d->digits[0] : 0);
        bigint_div(x, y, q);
        Bigint *t = x;
        x = q;
        q = t;
    }
    for (size_t j = 0; j < i / 2; j++) {
        char32_t t = chars[j];
        chars[j] = chars[i - j - 1];
        chars[i - j - 1] = t;
    }
    free(y);
    free(d);
    free(x);
    free(q);
    return i;
}
