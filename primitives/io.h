#include "../expr.h"

Val display_prim(Val *args, int num);
Val newline_prim(Val *args, int num);
Val error_prim(Val *args, int num);
void read_prim(int num, int *pc, Global_env **global_env);
