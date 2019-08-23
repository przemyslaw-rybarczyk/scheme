#include <stdio.h>
#include <stdlib.h>

#include "safestd.h"
#include "types.h"

/* == safestd.c
 * This file contains versions of `malloc`, `realloc`, `ungetc`, `fopen` and `fgetc`
 * that don't return error values and instead exit the program upon error.
 */

void *s_malloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        eprintf("Error: out of memory\n");
        exit(1);
    }
    return p;
}

void *s_realloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (p == NULL) {
        eprintf("Error: out of memory\n");
        exit(1);
    }
    return p;
}

void s_ungetc(char c, FILE *f) {
    if (ungetc(c, f) == EOF) {
        eprintf("Error: ungetc failed\n");
        exit(1);
    }
}

FILE *s_fopen(const char *pathname, const char *mode) {
    FILE *f = fopen(pathname, mode);
    if (f == NULL) {
        eprintf("Error: could not open file %s\n", pathname);
        exit(1);
    }
    return f;
}

int s_fgetc(FILE *f) {
    int c = getc(f);
    if (c == EOF && ferror(f)) {
        eprintf("Error: could not read file\n");
        exit(1);
    }
    return c;
}
