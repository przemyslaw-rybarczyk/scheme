#include <stdlib.h>
#include <string.h>

#include "symbol.h"
#include "types.h"
#include "safestd.h"

size_t obarray_size = 256;
char **obarray;
char **obarray_end;

/* -- setup_obarray
 * Sets up the variables providing the obarray.
 * Should be called at the beginning of `main`.
 */
void setup_obarray(void) {
    obarray = s_malloc(obarray_size * sizeof(const char *));
    obarray_end = obarray;
}

/* -- intern_symbol
 * Given a symbol string, it attempts to find it in the obarray.
 * If it is already there, the found string is returned.
 * Otherwise, the symbol string is added to the obarray and returned back.
 */
char *intern_symbol(char *symbol) {
    for (char **obarray_ptr = obarray; obarray_ptr < obarray_end; obarray_ptr++)
        if (strcmp(symbol, *obarray_ptr) == 0)
            return *obarray_ptr;
    size_t index = obarray_end - obarray;
    if (index >= obarray_size) {
        obarray_size *= 2;
        obarray = s_realloc(obarray, obarray_size);
        obarray_end = obarray + index;
    }
    *obarray_end++ = symbol;
    return symbol;
}
