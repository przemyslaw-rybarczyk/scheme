#include "expr.h"

void display_val(struct val val);
void display_sexpr(struct sexpr *sexpr);
const char *sprint_type(enum types);
void inner_display_val(struct val val);
void display_inst(struct inst *inst);
