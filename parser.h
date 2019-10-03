#include "types.h"

int32_t fgetc32_nospace(FILE *f);
int parser_init(FILE *f);
Val get_token(FILE *f);
uint32_t read_expr(FILE *f);
