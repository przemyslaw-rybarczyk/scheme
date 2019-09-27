#include "consts.h"
#include "types.h"
#include "safestd.h"

Val *constant_table;
static uint32_t constant_table_capacity = 256;
static uint32_t constant_table_size = 0;

void setup_constant_table(void) {
    constant_table = s_malloc(constant_table_capacity * sizeof(Val));
}

size_t add_constant(Val val) {
    if (constant_table_size == constant_table_capacity) {
        constant_table_capacity *= 2;
        constant_table = s_realloc(constant_table, constant_table_capacity * sizeof(Val));
    }
    constant_table[constant_table_size] = val;
    return constant_table_size++;
}
