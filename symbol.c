#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "symbol.h"
#include "types.h"
#include "safestd.h"
#include "string.h"

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

/* -- intern_symbol
 * Given a symbol string, it attempts to find it in the obarray.
 * If it is already there, the found string is returned and the argument freed.
 * Otherwise, the symbol string is added to the obarray and returned back.
 */
String *intern_symbol(String *symbol) {
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
