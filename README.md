# scheme

A basic implementation of Scheme in C that I initially made as exercise 5.51 from [Structure and Interpretation of Computer Programs](https://mitpress.mit.edu/sites/default/files/sicp/index.html).

## Compilation

To compile the interpreter, run `./compile.sh`.

### Flags
- `GC_ALWAYS` makes the garbage collector activate on every allocation. Useful for debugging.
- `SHOW_VM_CODE` shows the compiled VM code after inputting an expression.

## Progress

The completion status of the interpreter, grouped by chapters of R5RS which it complies with.

- 4. Expressions
    - █████░ 4.1. Primitive expression types
    - 4.2. Derived expression types
        - ████░░ 4.2.1. Conditionals
        - ██░░░░ 4.2.2. Binding constructs
        - ██████ 4.2.3. Sequencing
        - ░░░░░░ 4.2.4. Iteration
        - ░░░░░░ 4.2.5. Delayed evaluation
        - ░░░░░░ 4.2.6. Quasiquotation
    - ░░░░░░ 4.3. Macros
- 5. Program structure
    - ███░░░ 5.1. Programs
    - ██░░░░ 5.2. Definitions
    - ░░░░░░ 5.3. Internal definitions
- 6. Standard procedures
    - ████░░ 6.1. Equivalence predicates
    - █░░░░░ 6.2. Numbers
    - 6.3. Other data types
        - █████░ 6.3.1. Booleans
        - ███░░░ 6.3.2. Pairs and lists
        - ████░░ 6.3.3. Symbols
        - ░░░░░░ 6.3.4. Characters
        - █░░░░░ 6.3.5. Strings
        - ░░░░░░ 6.3.6. Vectors
    - ░░░░░░ 6.4. Control features
    - ░░░░░░ 6.5. `eval`
    - ░░░░░░ 6.6. Input and output
