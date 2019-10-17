#include "bigint.h"

#define BIGINT_ADD_LEN(x, y) ((bilabs((x)->len) > bilabs((y)->len) ? bilabs((x)->len) : bilabs((y)->len)) + 1)
#define BIGINT_SUB_LEN(x, y) BIGINT_ADD_LEN(x, y)
#define BIGINT_MUL_LEN(x, y) ((x)->len == 0 || (y)->len == 0 ? 0 : bilabs((x)->len) + bilabs((y)->len))
#define BIGINT_DIV_LEN(x, y) ((x)->len < (y)->len ? 0 : 1 + bilabs((x)->len) - bilabs((y)->len))

Bigint *bigint_add(Bigint *x, Bigint *y, Bigint *r);
Bigint *bigint_sub(Bigint *x, Bigint *y, Bigint *r);
Bigint *bigint_mul(Bigint *x, Bigint *y, Bigint *r);
Bigint *bigint_div(Bigint *x, Bigint *y, Bigint *r);
