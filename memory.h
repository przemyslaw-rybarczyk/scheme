#include "expr.h"

void setup_memory();
void *gc_alloc(size_t size);
void gc_push_env(Env **env);
void gc_pop_env();
