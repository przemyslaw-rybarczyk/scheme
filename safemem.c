#include <stdio.h>
#include <stdlib.h>

/* == safemem.c
 * This file contains versions of `malloc` and `realloc` that don't return
 * NULL pointers, and instead crash with an error message on running out of
 * memory.
 */

void *s_malloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        exit(3);
    }
    return p;
}

void *s_realloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        exit(3);
    }
    return p;
}

void s_ungetc(char c, FILE *stream) {
    if (ungetc(c, stdin) == EOF) {
        fprintf(stderr, "Error: ungetc failed\n");
        exit(3);
    }
}
