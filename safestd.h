#include <stdio.h>

void *s_malloc(size_t size);
void *s_realloc(void *ptr, size_t size);
void s_ungetc(char c, FILE *stream);
FILE *s_fopen(const char *pathname, const char *mode);
int s_fgetc(FILE *f);
void s_fputc(int c, FILE *f);
void s_fputs(const char *s, FILE *f);
void s_fgets(char *s, int size, FILE *f);
