#include <stdio.h>

#include "env.h"
#include "parser.h"
#include "eval.h"
#include "display.h"
#include "memory.h"
#include "symbol.h"

#ifndef COMPILED

const char *input_prompt = ">>> ";

/* -- main
 * Sets up the necessary variables and runs the REPL.
 */
int main() {
    setup_memory();
    setup_global_env();
    setup_obarray();
    printf("%s", input_prompt);
    struct expr *program = read();
    while (program != NULL) {
        struct val val = eval(program, NULL);
        if (val.type != TYPE_VOID) {
            display_val(val);
            putchar('\n');
        }
        printf(input_prompt);
        program = read();
    }
    putchar('\n');
}

#endif
