#include "types.h"

Global_env *compiler_env;
Global_env *make_global_env(int include_r5rs, int include_compiler);
void setup_env(void);
Val locate_var(Env_loc var, Env *env, Global_env *global);
uint32_t locate_global_var(char *var, Global_env *global);
void assign_var(Env_loc var, Val val, Env *env, Global_env *global);
void define_var(char *var, Val val, Global_env *global);
Env *extend_env(Val *vals_start, uint32_t vals_num, Env *env);
