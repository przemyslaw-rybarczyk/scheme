#include <stdint.h>
#include <uchar.h>

#include "unicode_data.h"

/* == unicode.c
 * TODO DOCUMENT
 */

static uint8_t get_short_properties(char32_t c) {
    if (c >= 0x30000)
        return 0;
    uint8_t row_num = row_index[c >> 5];
    if (row_num >= 0xF0)
        return row_num & 0x0F;
    uint8_t short_data;
    if (row_num < table_switch_point)
        short_data = row_table_index[c >> 15].short_row_table_ptr[row_num][(c >> 1) & 0x0F];
    else
        short_data = row_table_index[c >> 15].long_row_table_ptr[row_num - table_switch_point].short_data[(c >> 1) & 0x0F];
    if (c & 1)
        return short_data & 0x0F;
    else
        return short_data >> 4;
}

int is_lowercase(char32_t c) {
    return (get_short_properties(c) & 0x03) == 0x01;
}

int is_uppercase(char32_t c) {
    return (get_short_properties(c) & 0x03) == 0x02;
}

int is_alphabetic(char32_t c) {
    return (get_short_properties(c) & 0x03) != 0x00;
}

int is_numeric(char32_t c) {
    return (get_short_properties(c) >> 2) & 1;
}

int is_whitespace(char32_t c) {
    return (get_short_properties(c) >> 3) & 1;
}

static char32_t get_long_property(char32_t c, int prop) {
    if (c >= 0x30000)
        return c;
    uint8_t row_num = row_index[c >> 5];
    if (row_num >= 0xF0 || row_num < table_switch_point)
        return c;
    return row_table_index[c >> 15].long_row_table_ptr[row_num - table_switch_point].long_data[c & 0x1F][prop];
}

char32_t to_uppercase(char32_t c) {
    return get_long_property(c, 0);
}

char32_t to_lowercase(char32_t c) {
    return get_long_property(c, 1);
}

char32_t fold_case(char32_t c) {
    return get_long_property(c, 2);
}
