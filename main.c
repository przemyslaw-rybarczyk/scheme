#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "parser.h"
#include "display.h"
#include "memory.h"
#include "symbol.h"
#include "insts.h"
#include "exec.h"

const char *input_prompt = ">>> ";

/* -- main
 * Sets up the necessary variables and runs the REPL.
 */
int main(int argc, char **argv) {
    int compile_flag = 0;
    int load_flag = 0;
    setup_memory();
    setup_obarray();
    setup_insts();
    global_env = make_global_env();
    if (argc == 2 && strcmp(argv[1], "--compile") == 0)
        compile_flag = 1;
    else if (argc == 2 && strcmp(argv[1], "--load") == 0)
        load_flag = 1;
    else if (argc != 1) {
        fprintf(stderr, "Error: invalid arguments\n");
        exit(1);
    }
    if (!compile_flag)
        printf("%s", input_prompt);
    if (load_flag) {
        int program = this_inst();
        load_insts(fopen("compiled.sss", "rb"));
#ifdef SHOW_VM_CODE
        for (int inst = program; inst < this_inst() - 1; inst++)
            display_inst(inst);
#endif
        while (insts[program].type != INST_EOF) {
            Val val = exec(program);
            if (val.type != TYPE_VOID) {
                display_val(val);
                putchar('\n');
            }
            program = next_expr(program + 1);
        }
        return 0;
    }
    int program = this_inst();
    int expr = read_expr();
    while (expr != -1) {
#ifdef SHOW_VM_CODE
        for (int inst = expr; inst < this_inst(); inst++)
            display_inst(inst);
#endif
        if (!compile_flag) {
            Val val = exec(expr);
            if (val.type != TYPE_VOID) {
                display_val(val);
                putchar('\n');
            }
            printf("%s", input_prompt);
        }
        expr = read_expr();
    }
    if (compile_flag) {
        FILE *compiled = fopen("compiled.sss", "wb");
        save_magic(compiled);
        save_insts(compiled, program, this_inst());
    } else {
        putchar('\n');
    }
}
