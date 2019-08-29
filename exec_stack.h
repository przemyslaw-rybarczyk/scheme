#include "types.h"

/* == exec_stack.h
 * This header file provides direct access to the stack.
 * It is only included in files containing definitions of primitives
 * which require these to operate.
 */

Val *stack_ptr;
void stack_push(Val val);
Val stack_pop(void);
