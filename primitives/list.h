#include "../types.h"

Val list_q_prim(Val *args, uint32_t num);
Val list_prim(Val *args, uint32_t num);
Val length_prim(Val *args, uint32_t num);
Val append_prim(Val *args, uint32_t num);
Val reverse_prim(Val *args, uint32_t num);
Val list_tail_prim(Val *args, uint32_t num);
Val list_ref_prim(Val *args, uint32_t num);
Val memq_prim(Val *args, uint32_t num);
Val memv_prim(Val *args, uint32_t num);
Val member_prim(Val *args, uint32_t num);
Val assq_prim(Val *args, uint32_t num);
Val assv_prim(Val *args, uint32_t num);
Val assoc_prim(Val *args, uint32_t num);
High_prim_return apply_prim(uint32_t num);
