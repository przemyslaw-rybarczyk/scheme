#include <stdio.h>
#include "types.h"

Inst *insts;
uint32_t return_inst;
uint32_t tail_call_inst;
uint32_t map_continue_inst;
uint32_t compiler_pc;
uint32_t compile_pc;
uint32_t parse_pc;
void setup_insts(void);
uint32_t next_inst(void);
uint32_t this_inst(void);
uint32_t next_expr(uint32_t start);
void save_insts(FILE *fp, uint32_t start, uint32_t end);
void load_insts(FILE *fp);
