#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "../types.h"
#include "../display.h"

void type_error(Val val) {
    eprintf("Error: incorrect argument type - %s\n",
            type_name(val.type));
    exit(1);
}

void args_assert(int assertion) {
    if (!assertion) {
        eprintf("Error: invalid number of arguments\n");
        exit(1);
    }
}
