#include "../types.h"

Val make_vector_prim(Val *args, uint32_t num);
Val vector_prim(Val *args, uint32_t num);
Val vector_length_prim(Val *args, uint32_t num);
Val vector_ref_prim(Val *args, uint32_t num);
Val vector_set_prim(Val *args, uint32_t num);
Val vector_to_list_prim(Val *args, uint32_t num);
Val list_to_vector_prim(Val *args, uint32_t num);
Val vector_fill_prim(Val *args, uint32_t num);
