#include <uchar.h>

#define MAX_FULL_CASING_LENGTH 3

int is_lowercase(char32_t c);
int is_uppercase(char32_t c);
int is_alphabetic(char32_t c);
int is_numeric(char32_t c);
int is_whitespace(char32_t c);
int is_control(char32_t c);
char32_t to_uppercase(char32_t c);
char32_t to_lowercase(char32_t c);
char32_t fold_case(char32_t c);
uint16_t *full_uppercase(char32_t c);
uint16_t *full_lowercase(char32_t c);
uint16_t *full_foldcase(char32_t c);
