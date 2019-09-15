#include "../types.h"

int eq(Val val1, Val val2);
int eqv(Val val1, Val val2);
int equal(Val val1, Val val2);
Val eq_prim(Val *args, uint32_t num);
Val eqv_prim(Val *args, uint32_t num);
Val equal_prim(Val *args, uint32_t num);
Val pair_prim(Val *args, uint32_t num);
Val null_prim(Val *args, uint32_t num);
Val number_prim(Val *args, uint32_t num);
Val symbol_prim(Val *args, uint32_t num);
Val string_prim(Val *args, uint32_t num);
Val procedure_prim(Val *args, uint32_t num);
Val boolean_prim(Val *args, uint32_t num);
Val char_prim(Val *args, uint32_t num);
Val not_prim(Val *args, uint32_t num);
