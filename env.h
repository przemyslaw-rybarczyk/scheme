#include "expr.h"

struct val *global_env;
int global_env_size;
void setup_global_env(void);
struct val locate_var(struct env_loc var, struct env *env);
int locate_global_var(char *var);
void assign_var(struct env_loc var, struct val val, struct env *env);
void define_var(char *var, struct val val, struct env *env);
struct env *extend_env(struct val *vals_start, int vals_num, struct env *env);
