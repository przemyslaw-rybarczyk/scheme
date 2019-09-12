#include <string.h>

#include "string.h"
#include "types.h"
#include "memory.h"
#include "safestd.h"

String *new_string(size_t len, char32_t *chars) {
    String *str = s_malloc(sizeof(String) + len * sizeof(char32_t));
    str->len = len;
    memcpy(str->chars, chars, len * sizeof(char32_t));
    return str;
}

String *new_gc_string(size_t len, char32_t *chars) {
    String *str = gc_alloc(sizeof(String) + len * sizeof(char32_t));
    str->len = len;
    memcpy(str->chars, chars, len * sizeof(char32_t));
    return str;
}

String *new_string_from_cstring(char *s) {
    size_t len = strlen(s);
    String *str = s_malloc(sizeof(String) + len * sizeof(char32_t));
    str->len = len;
    for (size_t i = 0; i < len; i++)
        str->chars[i] = (char32_t)s[i];
    return str;
}

int string_eq(String *str1, String *str2) {
    if (str1->len != str2->len)
        return 0;
    for (size_t i = 0; i < str1->len; i++)
        if (str1->chars[i] != str2->chars[i])
            return 0;
    return 1;
}
