#include "expr.h"

struct frame *global_env_frame;
void setup_global_env(void);
struct val lookup_var(char *var, struct env *env);
struct val assign_var(char *var, struct val val, struct env *env);
struct val define_var(char *var, struct val *val, struct env *env);
struct env *extend_env(struct param_list *vars, struct val *vals_start, int vals_num, struct env *env);
