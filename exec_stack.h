#include "types.h"

/* == exec_stack.h
 * This header file provides functions for direct access to the stack.
 * It is only included in files containing definitions of high primitives,
 * which require these to operate.
 */

void stack_push(Val val);
Val stack_pop(void);
