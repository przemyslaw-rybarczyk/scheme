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
#include "primitives.h"
#include "safestd.h"
#include "string.h"

/* -- input_mode
 * Possible values and corresponding command-line options:
 * - INPUT_INTERACTIVE / (default)
 * - INPUT_FILE / (if files provided)
 * - INPUT_BYTECODE / --bytecode
 */
enum input_mode {
    INPUT_INTERACTIVE,
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
    enum input_mode input_mode = INPUT_INTERACTIVE;
    enum output_mode output_mode = OUTPUT_INTERACTIVE;
    uint32_t input_files_num = 0;
    uint32_t input_file_names_capacity = 4;
    char **input_file_names = s_malloc(input_file_names_capacity * sizeof(char *));
    char *output_file_name = NULL;
    int show_bytecode = 0;

    for (char **p = argv + 1; *p != NULL; p++) {
        char *arg = *p;
        if (strcmp(arg, "--bytecode") == 0) {
            if (input_mode != INPUT_INTERACTIVE && input_mode != INPUT_FILE) {
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
            if (input_mode == INPUT_INTERACTIVE)
                input_mode = INPUT_FILE;
            if (input_files_num == input_file_names_capacity) {
                input_file_names_capacity *= 2;
                input_file_names = s_realloc(input_file_names, input_file_names_capacity * sizeof(char *));
            }
            input_file_names[input_files_num++] = arg;
        }
    }

    if (input_mode != INPUT_INTERACTIVE && input_files_num == 0) {
        eprintf("Error: no input files provided\n");
        return 1;
    }

    setup_memory();
    setup_obarray();
    setup_primitives();
    execution_env = make_global_env(1, 0);
    compiler_env = make_global_env(1, 1);
    setup_insts();
    setup_env();

    uint32_t file = 0;
    uint32_t program = this_inst();
    uint32_t expr;
    FILE *input_file;

    if (output_mode == OUTPUT_INTERACTIVE && input_mode == INPUT_INTERACTIVE)
        printf("%s", input_prompt);

    switch (input_mode) {
    case INPUT_INTERACTIVE:
        break;
    case INPUT_FILE:
        input_file = s_fopen(input_file_names[file++], "r");
        break;
    case INPUT_BYTECODE:
        input_file = s_fopen(input_file_names[file++], "rb");
        expr = this_inst() - 1;
        load_insts(input_file);
        break;
    }

    while (1) {
        switch (input_mode) {
        case INPUT_INTERACTIVE:
            expr = read_expr(stdin);
            break;
        case INPUT_FILE:
            expr = read_expr(input_file);
            while (expr == UINT32_MAX && file != input_files_num) {
                fclose(input_file);
                input_file = s_fopen(input_file_names[file++], "r");
                expr = read_expr(input_file);
            }
            break;
        case INPUT_BYTECODE:
            expr = next_expr(expr + 1);
            while (insts[expr].type == INST_EOF && file != input_files_num) {
                fclose(input_file);
                input_file = s_fopen(input_file_names[file++], "rb");
                expr = this_inst();
                load_insts(input_file);
            }
            if (insts[expr].type == INST_EOF)
                expr = UINT32_MAX;
            break;
        }

        if (expr == UINT32_MAX)
            break;

        if (show_bytecode) {
            uint32_t end = this_inst();
            for (uint32_t inst = expr; inst < end; inst++)
                print_inst(inst);
        }

        switch (output_mode) {
        case OUTPUT_INTERACTIVE: {
            Val val = exec(expr, execution_env);
            if (val.type != TYPE_VOID) {
                print_val(val);
                putchar('\n');
            }
            if (input_mode == INPUT_INTERACTIVE)
                printf("%s", input_prompt);
            break;
        }
        case OUTPUT_RUN:
            exec(expr, execution_env);
            break;
        case OUTPUT_BYTECODE:
            break;
        }
    }

    if (input_mode != INPUT_INTERACTIVE)
        fclose(input_file);

    if (output_mode == OUTPUT_BYTECODE) {
        FILE *output_file = s_fopen(output_file_name, "wb");
        uint32_t end = this_inst();
        if (input_mode == INPUT_BYTECODE)
            end--;
        save_insts(output_file, program, end);
    } else if (input_mode == INPUT_INTERACTIVE) {
        putchar('\n');
    }

}
