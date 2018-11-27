#include "../expr.h"

struct val add_prim(struct val *args, int num);
struct val sub_prim(struct val *args, int num);
struct val mul_prim(struct val *args, int num);
struct val div_prim(struct val *args, int num);
struct val equ_prim(struct val *args, int num);
struct val lt_prim(struct val *args, int num);
struct val gt_prim(struct val *args, int num);
