#include "../expr.h"

struct val eq_prim(struct val_list *args);
struct val equal_prim(struct val_list *args);
struct val pair_prim(struct val_list *args);
struct val null_prim(struct val_list *args);
struct val number_prim(struct val_list *args);
struct val symbol_prim(struct val_list *args);
struct val string_prim(struct val_list *args);
struct val not_prim(struct val_list *args);
