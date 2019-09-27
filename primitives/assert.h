#include "../types.h"

#define false_val ((Val){TYPE_BOOL, {.int_data = 0}});
#define true_val ((Val){TYPE_BOOL, {.int_data = 1}});

void type_error(Val val);
void args_assert(int assertion);
