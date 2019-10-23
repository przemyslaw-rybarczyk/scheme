#include "../types.h"

Val add_prim(Val *args, uint32_t num);
Val sub_prim(Val *args, uint32_t num);
Val mul_prim(Val *args, uint32_t num);
Val div_prim(Val *args, uint32_t num);
Val mod_prim(Val *args, uint32_t num);
Val quotient_prim(Val *args, uint32_t num);
Val gcd_prim(Val *args, uint32_t num);
Val equ_prim(Val *args, uint32_t num);
Val lt_prim(Val *args, uint32_t num);
Val gt_prim(Val *args, uint32_t num);
Val make_rectangular_prim(Val *args, uint32_t num);
