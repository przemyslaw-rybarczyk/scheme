#include <stdio.h>
#include "types.h"

#define EOF32 (-1)
#define putchar32(c) fputc32(c, stdout);
#define puts32(s) fputs32(s, stdout);
#define eputs32(s) fputs32(s, stderr);

void *s_malloc(size_t size);
void *s_realloc(void *ptr, size_t size);
FILE *s_fopen(const char *pathname, const char *mode);
void fputc32(char32_t c, FILE *f);
void fputs32(String *str, FILE *f);
int s_fgetc(FILE *f);
int32_t s_fgetc32(FILE *f);
void s_fputc(int c, FILE *f);
void s_fputs(const char *s, FILE *f);
void s_fgets(char *s, int size, FILE *f);
