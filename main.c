#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "display.h"
#include "env.h"
#include "exec.h"
#include "insts.h"
#include "memory.h"
#include "parser.h"
#include "safestd.h"
#include "symbol.h"

/* -- input_mode
 * Possible values and corresponding command-line options:
 * - INPUT_FILE / (default)
 * - INPUT_BYTECODE / --bytecode
 */
enum input_mode {
    INPUT_FILE,
    INPUT_BYTECODE,
};

/* -- output_mode
 * Possible values and corresponding command-line options:
 * - OUTPUT_INTERACTIVE / (default)
 * - OUTPUT_RUN / --run
 * - OUTPUT_BYTECODE / --compile
 */
enum output_mode {
    OUTPUT_INTERACTIVE,
    OUTPUT_RUN,
    OUTPUT_BYTECODE,
};

const char *input_prompt = ">>> ";

int main(int argc, char **argv) {
    setup_memory();
    setup_obarray();
    Global_env *global_env = make_global_env(1, 0);
    setup_insts();
    setup_env();

    enum input_mode input_mode = INPUT_FILE;
    enum output_mode output_mode = OUTPUT_INTERACTIVE;
    char *input_file_name = NULL;
    char *output_file_name = NULL;
    int show_bytecode = 0;

    for (char **p = argv + 1; *p != NULL; p++) {
        char *arg = *p;
        if (strcmp(arg, "--bytecode") == 0) {
            if (input_mode != INPUT_FILE) {
                eprintf("Error: multiple input modes specified\n");
                return 1;
            }
            input_mode = INPUT_BYTECODE;
        } else if (strcmp(arg, "--run") == 0) {
            if (output_mode != OUTPUT_INTERACTIVE) {
                eprintf("Error: multiple output modes specified\n");
                return 1;
            }
            output_mode = OUTPUT_RUN;
        } else if (strcmp(arg, "--compile") == 0) {
            if (output_mode != OUTPUT_INTERACTIVE) {
                eprintf("Error: multiple output modes specified\n");
                return 1;
            }
            output_mode = OUTPUT_BYTECODE;
            arg = *++p;
            if (arg == NULL || strncmp(arg, "--", 2) == 0) {
                eprintf("Error: no filename provided for --compile option\n");
                return 1;
            }
            if (output_file_name != NULL) {
                eprintf("Error: multiple output files specified\n");
                return 1;
            }
            output_file_name = arg;
        } else if (strcmp(arg, "--show-bytecode") == 0) {
            show_bytecode = 1;
        } else if (strncmp(arg, "--", 2) == 0) {
            eprintf("Error: invalid command-line option %s\n", arg);
            return 1;
        } else {
            if (input_file_name != NULL) {
                eprintf("Error: multiple input files specified\n");
                return 1;
            }
            input_file_name = arg;
        }
    }

    FILE *input_file = stdin;
    switch (input_mode) {
    case INPUT_FILE:
        if (input_file_name != NULL)
            input_file = s_fopen(input_file_name, "r");
        break;
    case INPUT_BYTECODE:
        if (input_file_name == NULL) {
            eprintf("Error: no bytecode input file specified\n");
            return 1;
        }
        input_file = s_fopen(input_file_name, "rb");
        break;
    }

    uint32_t program = this_inst();
    uint32_t expr;

    if (output_mode == OUTPUT_INTERACTIVE && input_file_name == NULL)
        printf("%s", input_prompt);

    switch (input_mode) {
    case INPUT_FILE:
        expr = read_expr(input_file);
        break;
    case INPUT_BYTECODE:
        load_insts(input_file);
        expr = program;
        if (insts[expr].type == INST_EOF)
            expr = UINT32_MAX;
        break;
    }

    while (expr != UINT32_MAX) {
        if (show_bytecode)
            for (uint32_t inst = expr; inst < this_inst(); inst++)
                print_inst(inst);

        switch (output_mode) {
        case OUTPUT_INTERACTIVE: {
            Val val = exec(expr, global_env);
            if (val.type != TYPE_VOID) {
                print_val(val);
                putchar('\n');
            }
            if (input_file_name == NULL)
                printf("%s", input_prompt);
            break;
        }
        case OUTPUT_RUN:
            exec(expr, global_env);
            break;
        case OUTPUT_BYTECODE:
            break;
        }

        switch (input_mode) {
        case INPUT_FILE:
            expr = read_expr(input_file);
            break;
        case INPUT_BYTECODE:
            expr = next_expr(expr + 1);
            if (insts[expr].type == INST_EOF)
                expr = UINT32_MAX;
            break;
        }
    }

    if (output_mode == OUTPUT_BYTECODE) {
        FILE *output_file = s_fopen(output_file_name, "wb");
        uint32_t end = this_inst();
        if (input_mode == INPUT_BYTECODE)
            end--;
        save_insts(output_file, program, end);
    } else {
        putchar('\n');
    }

}
