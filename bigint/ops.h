#include "bigint.h"

#define BIGINT_ADD_LEN(x, y) ((bilabs((x)->len) > bilabs((y)->len) ? bilabs((x)->len) : bilabs((y)->len)) + 1)
#define BIGINT_SUB_LEN(x, y) BIGINT_ADD_LEN(x, y)
#define BIGINT_MUL_LEN(x, y) ((x)->len == 0 || (y)->len == 0 ? 0 : bilabs((x)->len) + bilabs((y)->len))
#define BIGINT_DIV_LEN(x, y) ((x)->len < (y)->len ? 0 : 1 + bilabs((x)->len) - bilabs((y)->len))
#define BIGINT_MOD_LEN(x, y) bilabs((y)->len)
#define BIGINT_GCD_LEN(x, y) (bilabs((x)->len) < bilabs((y)->len) ? bilabs((x)->len) : bilabs((y)->len))

/* == bigint/ops.h
 * These functions require a bigint (r) of sufficient size to write the result to.
 * The macros BIGINT_<op>_LEN can be used to get the necessary number of
 * digits of a bigint. The allocation is done outside the functions so that
 * the result may be stored either on the heap or in garbage-collected memory.
 * Note that some operations may require more allocated digits than the result
 * acually has, so the macros should always be used.
 */

void bigint_add(Bigint *x, Bigint *y, Bigint *r);
void bigint_sub(Bigint *x, Bigint *y, Bigint *r);
void bigint_mul(Bigint *x, Bigint *y, Bigint *r);
void bigint_div(Bigint *x, Bigint *y, Bigint *r);
void bigint_mod(Bigint *x, Bigint *y, Bigint *r);
void bigint_gcd(Bigint *x, Bigint *y, Bigint *r);
