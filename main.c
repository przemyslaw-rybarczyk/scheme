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
    int program = read_expr();
    FILE *compiled = compile_flag ? fopen("compiled.sss", "wb") : NULL;
    if (compile_flag)
        save_magic(compiled);
    while (program != -1) {
#ifdef SHOW_VM_CODE
        for (int inst = program; inst < this_inst(); inst++)
            display_inst(inst);
#endif
        if (compile_flag)
            save_insts(compiled, program, this_inst());
        else {
            Val val = exec(program);
            if (val.type != TYPE_VOID) {
                display_val(val);
                putchar('\n');
            }
        }
        if (!compile_flag)
            printf("%s", input_prompt);
        program = read_expr();
    }
    putchar('\n');
}
