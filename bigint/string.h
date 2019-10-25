#include "bigint.h"

Bigint *read_bigint_hexadecimal(size_t len, char32_t *chars);
size_t bigint_sprint_base(Bigint *bi, bi_base base, char32_t *chars);
