#include <stdio.h>
#include <stdlib.h>

#include "../expr.h"
#include "../display.h"

void type_error(struct val val) {
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

void assert_1_arg(struct val_list *args) {
    args_assert(args != NULL && args->cdr == NULL);
}

void assert_2_args(struct val_list *args) {
    args_assert(args != NULL && args->cdr != NULL && args->cdr->cdr == NULL);
}

