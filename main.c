#include <stdio.h>

#include "env.h"
#include "parser.h"
#include "display.h"
#include "memory.h"
#include "symbol.h"
#include "insts.h"
#include "compile.h"
#include "exec.h"

const char *input_prompt = ">>> ";

/* -- main
 * Sets up the necessary variables and runs the REPL.
 */
int main() {
    setup_insts();
    setup_memory();
    setup_global_env();
    setup_obarray();
    printf("%s", input_prompt);
    struct expr *ast = read();
    struct inst *program;
    while (ast != NULL) {
        program = this_inst();
        compile(ast, 1);
#ifdef SHOW_VM_CODE
        for (struct inst *inst = program; inst < this_inst(); inst++)
            display_inst(inst);
#endif
        struct val val = exec(program);
        if (val.type != TYPE_VOID) {
            display_val(val);
            putchar('\n');
        }
        printf(input_prompt);
        ast = read();
    }
    putchar('\n');
}
