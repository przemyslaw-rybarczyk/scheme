#include "expr.h"

struct val eval(struct expr *expr, struct env *env);
int is_true(struct val val);
struct val apply(struct val proc, struct val_list *args);
