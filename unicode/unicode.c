#include <stdint.h>
#include <uchar.h>

#include "unicode_data.h"

/* == unicode.c
 * The Unicode data is stored in several arrays in order to implement some
 * basic compression. The tables only contain data up to U+2FFFF, as later
 * code points do not have useful values assigned to them yet.
 *
 * The Unicode code points are split into 'rows' of 32 characters each.
 * The row index contains data about the Unicode properties of each row.
 * The rows where all characters have the same binary properties and have no
 * casing variants are represented in the row index as bytes with their first
 * four bits set (240 to 255). The lower four bits represent the common binary
 * property values.
 *
 * Other values represent indices of entries in one of the row tables. Values
 * less than table_switch_point correspond to short rows and the remaining
 * values correspond to long rows. A row is represented as a short row if none
 * of its code points have casing variants. The binary properties are then
 * stored as 16 bytes, each one containing values for two code points.
 *
 * A long row contains the same data as a short row, along with 3 16-bit
 * integers for each code point in the row, representing the lower 16 bits
 * of the uppercase, lowercase, and case-folded variants respectively. Since
 * these always lie in the same Unicode plane as the original code point,
 * there is no need to represent the remaining bits.
 *
 * Since there are more entries in the tables than can be indexed by a single
 * byte, the indices given in the row index table are relative to the first row
 * of a block of size 0x8000 contained in each table. The pointers to these rows
 * are found the row table index.
 *
 * Of the four bits representing binary property data, the first two represent
 * the White_Space and Numeric_Type=Decimal properties. The remaining two
 * represent the various types of alphabetic characters. Their four possible
 * values (00, 01, 10, 11) represent respectively non-Alphabetic, Lowercase,
 * Uppercase, and Alphabetic but neither Lowercase nor Uppercase. Note that
 * Lowercase and Uppercase are both subsets of Alphabetic and do not intersect,
 * allowing for this kind of representation.
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
    return (c & 0xFF0000) | row_table_index[c >> 15].long_row_table_ptr[row_num - table_switch_point].long_data[c & 0x1F][prop];
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
