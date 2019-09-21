#include "types.h"

void setup_obarray(void);
String *intern_string(String *str);
String *gc_alloc_string(size_t len);
String *new_interned_string_from_cstring(char *s);
int string_eq(String *str1, String *str2);
