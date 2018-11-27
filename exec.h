#include "expr.h"

extern struct val stack[];
struct val *stack_ptr;
struct env *exec_env;

void stack_push(struct val val);
struct val stack_pop(void);

struct val exec(struct inst *inst);
int is_true(struct val val);
