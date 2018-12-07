#include <stdio.h>
#include "expr.h"

struct inst *insts;
int return_inst;
int tail_call_inst;
void setup_insts(void);
int next_inst(void);
int this_inst(void);
int next_expr(int start);
void save_insts(FILE *fp, int start, int end);
void load_insts(FILE *fp);
