#include <stdlib.h>
#include <string.h>
#include <uchar.h>

#include "bigint.h"
#include "../safestd.h"

bi_base read_digit(char32_t c) {
    if (c >= 'a')
        return c - 'a' + 10;
    if (c >= 'A')
        return c - 'A' + 10;
    return c - '0';
}

char write_digit(char i) {
    if (i >= 10)
        return i - 10 + 'a';
    return i + '0';
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

void print_bigint_hexadecimal(Bigint *bi) {
    size_t len;
    if (bi->len < 0) {
        printf("-");
        len = (size_t)(-bi->len);
    } else {
        len = (size_t)bi->len;
    }
    printf("#x");
    for (size_t i = len; i-- > 0; ) {
        for (int j = HEX_CPD; j-- > 0; )
            printf("%c", write_digit((char)((bi->digits[i] >> (4 * j)) & 0xF)));
    }
}
