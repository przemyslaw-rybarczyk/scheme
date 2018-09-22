#include "../expr.h"

struct val add_prim(struct val_list *args);
struct val sub_prim(struct val_list *args);
struct val mul_prim(struct val_list *args);
struct val div_prim(struct val_list *args);
struct val equ_prim(struct val_list *args);
struct val lt_prim(struct val_list *args);
struct val gt_prim(struct val_list *args);
