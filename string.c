#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "string.h"
#include "types.h"
#include "memory.h"
#include "safestd.h"

static size_t obarray_size = 256;
static String **obarray;
static String **obarray_end;

/* -- setup_obarray
 * Sets up the variables providing the obarray.
 * Should be called at the beginning of `main`.
 */
void setup_obarray(void) {
    obarray = s_malloc(obarray_size * sizeof(String *));
    obarray_end = obarray;
}

/* -- intern_string
 * Given a string, it attempts to find it in the obarray.
 * If it is already there, the found string is returned and the argument freed.
 * Otherwise, the symbol string is added to the obarray and returned back.
 */
String *intern_string(String *symbol) {
    for (String **obarray_ptr = obarray; obarray_ptr < obarray_end; obarray_ptr++) {
        if (string_eq(symbol, *obarray_ptr)) {
            free(symbol);
            return *obarray_ptr;
        }
    }
    ptrdiff_t index = obarray_end - obarray;
    if (index >= obarray_size) {
        obarray_size *= 2;
        obarray = s_realloc(obarray, obarray_size * sizeof(String *));
        obarray_end = obarray + index;
    }
    *obarray_end++ = symbol;
    return symbol;
}

static int string_eq_buf(String *str, size_t len, char32_t *chars);

/* -- new_interned_string
 * Works like intern_string, except it takes the length and characters as separate arguments.
 * The string is not freed if it already exists in the obarray, and is copied otherwise.
 */
String *new_interned_string(size_t len, char32_t *chars) {
    for (String **obarray_ptr = obarray; obarray_ptr < obarray_end; obarray_ptr++) {
        if (string_eq_buf(*obarray_ptr, len, chars))
            return *obarray_ptr;
    }
    ptrdiff_t index = obarray_end - obarray;
    if (index >= obarray_size) {
        obarray_size *= 2;
        obarray = s_realloc(obarray, obarray_size * sizeof(String *));
        obarray_end = obarray + index;
    }
    String *interned = s_malloc(sizeof(String) + len * sizeof(char32_t));
    interned->len = len;
    memcpy(interned->chars, chars, len * sizeof(char32_t));
    *obarray_end++ = interned;
    return interned;
}

String *gc_alloc_string(size_t len) {
    String *str = gc_alloc(sizeof(String) + (len ? len : 1) * sizeof(char32_t));
    str->len = len;
    str->chars[0] = 0;
    return str;
}

String *new_interned_string_from_cstring(char *s) {
    size_t len = strlen(s);
    String *str = s_malloc(sizeof(String) + len * sizeof(char32_t));
    str->len = len;
    for (size_t i = 0; i < len; i++)
        str->chars[i] = (char32_t)s[i];
    return intern_string(str);
}

int string_eq(String *str1, String *str2) {
    if (str1->len != str2->len)
        return 0;
    for (size_t i = 0; i < str1->len; i++)
        if (str1->chars[i] != str2->chars[i])
            return 0;
    return 1;
}

static int string_eq_buf(String *str, size_t len, char32_t *chars) {
    if (str->len != len)
        return 0;
    for (size_t i = 0; i < len; i++)
        if (str->chars[i] != chars[i])
            return 0;
    return 1;
}
