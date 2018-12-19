# scheme

A basic implementation of Scheme in C that I initially made as exercise 5.51 from [Structure and Interpretation of Computer Programs](https://mitpress.mit.edu/sites/default/files/sicp/index.html).

## Compilation

### Flags
- `GC_ALWAYS` makes the garbage collector activate on every allocation. Useful for debugging.
- `SHOW_VM_CODE` shows the compiled VM code after inputting an expression.
