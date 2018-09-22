# sicp-5-51

A basic implementation of Scheme in C, done as exercise 5.51 from Structure and Interpretation of Computer Programs.

## Compilation
Tail recursion optimization requires compiling with the appropriate settings (`-O2` on gcc).

### Flags
- `GC_ALWAYS` makes the garbage collector activate on every allocation. Useful for debugging.
- `COMPILED` is only used with code compiled by my sicp-5-52 compiler.
