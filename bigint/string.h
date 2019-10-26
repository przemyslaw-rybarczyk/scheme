#include "bigint.h"
#include "../types.h"

size_t bigint_write_base(Bigint *bi, bi_base base, char32_t *chars);
int bigint_read_base(size_t len, char32_t *chars, bi_base base, Bigint *r);
