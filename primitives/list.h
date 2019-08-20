#include "../expr.h"

Val list_prim(Val *args, int num);
Val length_prim(Val *args, int num);
void apply_prim(int num, int *pc, Global_env **global_env);
