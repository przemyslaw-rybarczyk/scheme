# scheme

A basic implementation of Scheme in C that I initially made as exercise 5.51 from Structure and Interpretation of Computer Programs.

## Compilation
Tail recursion optimization requires compiling with the appropriate settings (`-O2` on gcc).

### Flags
- `GC_ALWAYS` makes the garbage collector activate on every allocation. Useful for debugging.
