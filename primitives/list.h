#include "../expr.h"

struct val list_prim(struct val *args, int num);
struct val length_prim(struct val *args, int num);
struct inst *apply_prim(int num);
