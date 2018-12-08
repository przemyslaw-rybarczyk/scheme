#include "expr.h"

Val *global_env;
int global_env_size;
void setup_global_env(void);
Val locate_var(Env_loc var, Env *env);
int locate_global_var(char *var);
void assign_var(Env_loc var, Val val, Env *env);
void define_var(char *var, Val val, Env *env);
Env *extend_env(Val *vals_start, int vals_num, Env *env);
