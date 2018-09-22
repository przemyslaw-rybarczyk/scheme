#include "expr.h"

struct val eval(struct expr *expr, struct env *env);
struct val apply(struct val proc, struct val_list *args);
struct val eval_quote(struct sexpr *quote);
void free_arg_list(struct val_list *list);
int is_true(struct val val);
