#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "../expr.h"
#include "../display.h"

void type_error(Val val) {
    fprintf(stderr, "Error: incorrect argument type - %s\n",
            sprint_type(val.type));
    exit(2);
}

void args_error(void) {
    fprintf(stderr, "Error: invalid number of arguments\n");
    exit(2);
}

void args_assert(int assertion) {
    if (assertion)
        return;
    args_error();
}
