#include "types.h"

void setup_memory(void);
void *gc_alloc(size_t size);
void gc_lock_env(Env **env_ptr);
void gc_unlock_env(void);
