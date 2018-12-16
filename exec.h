#include "expr.h"

extern Val stack[];
Val *stack_ptr;
Global_env *global_env;
Env *exec_env;

void stack_push(Val val);
Val stack_pop(void);

Val exec(long init_pc);
int is_true(Val val);
