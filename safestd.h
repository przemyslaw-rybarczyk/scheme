#include <stdio.h>

void *s_malloc(size_t size);
void *s_realloc(void *ptr, size_t size);
void s_ungetc(char c, FILE *stream);
FILE *s_fopen(const char *pathname, const char *mode);
int s_fgetc(FILE *f);
