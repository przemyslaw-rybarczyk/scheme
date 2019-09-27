#include <stdio.h>
#include "../types.h"

FILE *compiler_input_file;

Val next_token_prim(Val *args, uint32_t num);
Val this_inst_prim(Val *args, uint32_t num);
Val next_inst_prim(Val *args, uint32_t num);
Val set_const_prim(Val *args, uint32_t num);
Val set_var_prim(Val *args, uint32_t num);
Val set_name_prim(Val *args, uint32_t num);
Val set_def_prim(Val *args, uint32_t num);
Val set_set_prim(Val *args, uint32_t num);
Val set_set_name_prim(Val *args, uint32_t num);
Val set_jump_prim(Val *args, uint32_t num);
Val set_jump_false_prim(Val *args, uint32_t num);
Val set_lambda_prim(Val *args, uint32_t num);
Val set_call_prim(Val *args, uint32_t num);
Val set_tail_call_prim(Val *args, uint32_t num);
Val set_return_prim(Val *args, uint32_t num);
Val set_delete_prim(Val *args, uint32_t num);
Val set_cons_prim(Val *args, uint32_t num);
