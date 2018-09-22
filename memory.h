#include "expr.h"

void setup_memory();

struct pair *alloc_pair(void);
struct lambda *alloc_lambda(void);
struct frame *alloc_frame(void);
struct env *alloc_env(void);

struct env **gc_push_env(struct env *env);
void gc_pop_env(void);
void gc_push_val(struct val *val);
void gc_pop_val(void);
