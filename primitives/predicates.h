#include "../types.h"

Val eq_prim(Val *args, uint32_t num);
Val equal_prim(Val *args, uint32_t num);
Val pair_prim(Val *args, uint32_t num);
Val null_prim(Val *args, uint32_t num);
Val number_prim(Val *args, uint32_t num);
Val symbol_prim(Val *args, uint32_t num);
Val string_prim(Val *args, uint32_t num);
Val procedure_prim(Val *args, uint32_t num);
Val not_prim(Val *args, uint32_t num);
