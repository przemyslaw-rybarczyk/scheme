# scheme

A basic implementation of Scheme in C that I initially made as exercise 5.51 from [Structure and Interpretation of Computer Programs](https://mitpress.mit.edu/sites/default/files/sicp/index.html).

## Compilation
To compile the interpreter, run `./compile.sh`.
The `$CC` environment variable can be used to change used C compiler used. `gcc` is used by default.

### Flags
- `GC_ALWAYS` makes the garbage collector activate on every allocation. Useful for debugging.
- `LOAD_FROM_CURRENT_DIR` disables the code that attempts to locate the executable and always loads the required bytecode files from the working directory.

## Usage
`./scheme [FILE] [--run] [--compile FILE] [--bytecode] [--show-bytecode]`

### Flags
- `--bytecode` - Reads the file as compiled bytecode, rather than a Scheme file.
- `--compile FILE` - Compiles the file and saves the bytecode to the given file.
- `--run` - Disables displaying of values of top-level expressions.
- `--show-bytecode` - Shows the compiled bytecode.

## Progress

The completion status of the interpreter, grouped by chapters of R5RS which it complies with.

- 4. Expressions
    - ██████ 4.1. Primitive expression types
    - 4.2. Derived expression types
        - ████░░ 4.2.1. Conditionals
        - ██░░░░ 4.2.2. Binding constructs
        - ██████ 4.2.3. Sequencing
        - ░░░░░░ 4.2.4. Iteration
        - ░░░░░░ 4.2.5. Delayed evaluation
        - ░░░░░░ 4.2.6. Quasiquotation
    - ░░░░░░ 4.3. Macros
- 5. Program structure
    - ██████ 5.1. Programs
    - ██████ 5.2. Definitions
    - ░░░░░░ 5.3. Syntax definitions
- 6. Standard procedures
    - ████░░ 6.1. Equivalence predicates
    - █░░░░░ 6.2. Numbers
    - 6.3. Other data types
        - ██████ 6.3.1. Booleans
        - ██████ 6.3.2. Pairs and lists
        - ████░░ 6.3.3. Symbols
        - ░░░░░░ 6.3.4. Characters
        - █░░░░░ 6.3.5. Strings
        - ░░░░░░ 6.3.6. Vectors
    - █░░░░░ 6.4. Control features
    - ░░░░░░ 6.5. `eval`
    - ░░░░░░ 6.6. Input and output
