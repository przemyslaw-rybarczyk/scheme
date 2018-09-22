#include "../expr.h"

void type_error(struct val val);
void args_error(void);
void args_assert(int assertion);
void assert_1_arg(struct val_list *args);
void assert_2_args(struct val_list *args);
