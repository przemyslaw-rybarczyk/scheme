#include "expr.h"

void stack_push(Val val);
Val stack_pop(void);
Val exec(int init_pc, Global_env *global_env);
int is_true(Val val);
