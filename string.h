#include "types.h"

String *new_string(size_t len, char32_t *chars);
String *new_gc_string(size_t len, char32_t *chars);
String *new_string_from_cstring(char *s);
int string_eq(String *str1, String *str2);
