#include "../expr.h"

struct val eq_prim(struct val *args, int num);
struct val equal_prim(struct val *args, int num);
struct val pair_prim(struct val *args, int num);
struct val null_prim(struct val *args, int num);
struct val number_prim(struct val *args, int num);
struct val symbol_prim(struct val *args, int num);
struct val string_prim(struct val *args, int num);
struct val not_prim(struct val *args, int num);
