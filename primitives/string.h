#include "../types.h"

Val make_string_prim(Val *args, uint32_t num);
Val string_prim(Val *args, uint32_t num);
Val string_length_prim(Val *args, uint32_t num);
Val string_ref_prim(Val *args, uint32_t num);
Val string_set_prim(Val *args, uint32_t num);
Val string_eq_prim(Val *args, uint32_t num);
Val string_lt_prim(Val *args, uint32_t num);
Val string_gt_prim(Val *args, uint32_t num);
Val string_le_prim(Val *args, uint32_t num);
Val string_ge_prim(Val *args, uint32_t num);
Val string_ci_eq_prim(Val *args, uint32_t num);
Val string_ci_lt_prim(Val *args, uint32_t num);
Val string_ci_gt_prim(Val *args, uint32_t num);
Val string_ci_le_prim(Val *args, uint32_t num);
Val string_ci_ge_prim(Val *args, uint32_t num);
