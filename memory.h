#include "expr.h"

void setup_memory();

struct pair *alloc_pair();
struct lambda *alloc_lambda();
struct frame *alloc_frame();
struct env *alloc_env();

struct env **gc_push_env(struct env *env);
void gc_pop_env();
void gc_push_val(struct val *val);
void gc_pop_val();
