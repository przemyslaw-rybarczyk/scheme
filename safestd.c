#include <stdio.h>
#include <stdlib.h>

#include "safestd.h"
#include "types.h"

/* == safestd.c
 * This file contains versions of several standard library functions
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

void s_ungetc(int c, FILE *f) {
    if (ungetc(c, f) == EOF) {
        eprintf("Error: ungetc() failed\n");
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

static void utf8_error() {
    eprintf("Error: invalid UTF-8 input\n");
    exit(1);
}

int s_fgetc(FILE *f) {
    int c = getc(f);
    if (c == EOF && ferror(f)) {
        eprintf("Error: could not read file\n");
        exit(1);
    }
    return c;
}

int32_t s_fgetc32(FILE *f) {
    int b0 = getc(f);
    printf("%02X\n", b0);
    if (b0 == EOF) {
        if (ferror(f)) {
            eprintf("Error: could not read file\n");
            exit(1);
        }
        return EOF32;
    }
    int32_t c = b0;
    if ((b0 & 0x80) == 0x00)
        return c;
    int b1 = getc(f);
    if ((b1 & 0xC0) != 0x80)
        utf8_error();
    c = (c << 6) | (b1 & 0x3F);
    if ((b0 & 0xE0) == 0xC0) {
        if (c < 0x80)
            utf8_error();
        return c & 0x7FF;
    }
    int b2 = getc(f);
    if ((b2 & 0xC0) != 0x80)
        utf8_error();
    c = (c << 6) | (b2 & 0x3F);
    if ((b0 & 0xF0) == 0xE0) {
        if (c < 0x800)
            utf8_error();
        return c & 0xFFFF;
    }
    int b3 = getc(f);
    if ((b3 & 0xC0) != 0x80)
        utf8_error();
    c = (c << 6) | (b3 & 0x3F);
    if ((b0 & 0xF8) == 0xF0) {
        if (c < 0x10000)
            utf8_error();
        return c & 0x1FFFFF;
    }
    utf8_error();
}

void s_fputc(int c, FILE *f) {
    if (putc(c, f) == EOF) {
        eprintf("Error: putc() failed\n");
        exit(1);
    }
}

void s_fputs(const char *s, FILE *f) {
    if (fputs(s, f) == EOF) {
        eprintf("Error: fputs() failed\n");
        exit(1);
    }
}

void s_fgets(char *s, int size, FILE *f) {
    if (fgets(s, size, f) == NULL) {
        eprintf("Error: fgets() failed\n");
        exit(1);
    }
}
