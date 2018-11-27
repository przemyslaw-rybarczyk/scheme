#include "../expr.h"

struct val display_prim(struct val *args, int num);
struct val newline_prim(struct val *args, int num);
struct val error_prim(struct val *args, int num);
struct inst *read_prim(int num);
