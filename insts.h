#include <stdio.h>
#include "expr.h"

Inst *insts;
int return_inst;
int tail_call_inst;
int compiler_pc;
int compile_pc;
int parse_pc;
void setup_insts(void);
int next_inst(void);
int this_inst(void);
int next_expr(int start);
void save_magic(FILE *fp);
void save_insts(FILE *fp, int start, int end);
void load_insts(FILE *fp);
