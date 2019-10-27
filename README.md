# scheme

A basic implementation of Scheme in C that I initially made as exercise 5.51 from [Structure and Interpretation of Computer Programs](https://mitpress.mit.edu/sites/default/files/sicp/index.html).

## Compilation
To compile the interpreter, run `make`. The C compiler can be set with `CC=`. `clang` is used by default.

### Flags
- `GC_ALWAYS` makes the garbage collector activate on every allocation. Useful for debugging.
- `LOAD_FROM_CURRENT_DIR` disables the code that attempts to locate the executable and always loads the required bytecode files from the working directory.
- `STACK_SIZE` sets the number of values that can fit on the stack. It is set to 65536 by default. Note that each non-tail recursion pushes two values to the stack.

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
    - ██████ 4.3. Macros
- ██████ 5. Program structure
- 6. Standard procedures
    - ████░░ 6.1. Equivalence predicates
    - █░░░░░ 6.2. Numbers
    - ██████ 6.3. Other data types
    - ██░░░░ 6.4. Control features
    - ░░░░░░ 6.5. `eval`
    - ░░░░░░ 6.6. Input and output
