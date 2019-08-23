#include "types.h"

/* == exec_gc.h
 * This header file exposes some internal variables of the stack machine.
 * It is only included in the garbage collector, which requires access to such details.
 */

extern Val stack[];
Val *stack_ptr;
Global_env *global_env;
Env *exec_env;
