#pragma once

/* -- val
 * Contains a Scheme value with `type` representing its type and data
 * containing the value.
 * The valid representations are:
 * - Integer - TYPE_INT / int_data
 * - Floating-point - TYPE_FLOAT / float_data
 * - Boolean - TYPE_BOOL / int_data
 * - String - TYPE_STRING / string_data
 * - Symbol - TYPE_SYMBOL / string_data
 * - Primitive - TYPE_PRIM / prim_data
 * - Higher-order primitive - TYPE_HIGH_PRIM / prim_data
 *   * Note: not all higher-order primitives are actually of type TYPE_HIGH_PRIM.
 * - Lambda - TYPE_LAMBDA / lambda_data
 * - Pair - TYPE_PAIR / pair_data
 * - Nil - TYPE_NIL / -unspecified-
 * - Void - TYPE_VOID / -unspecified-
 * Lambdas and pairs are represented as pointers to the heap.
 * Strings and symbols are immutable and may only be defined as literals.
 * A void type is used as a return value when none is specified, such as
 * in assignments and definitions.
 * A `type` value of TYPE_BROKEN_HEART is used only for garbage collection.
 * The following values are used only for returning from functions:
 * - Environment - TYPE_ENV / env_data
 * - Instruction pointer - TYPE_INST / inst_data
 */

enum types {TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_STRING, TYPE_SYMBOL,
    TYPE_PRIM, TYPE_HIGH_PRIM, TYPE_LAMBDA, TYPE_PAIR, TYPE_NIL, TYPE_VOID,
    TYPE_BROKEN_HEART, TYPE_ENV, TYPE_INST};

struct pair;
struct lambda;
struct val_list;

struct val {
    enum types type;
    union {
        long long int_data;
        double float_data;
        char *string_data;
        struct val (*prim_data)(struct val *, int);
        struct inst *(*high_prim_data)(int);
        struct lambda *lambda_data;
        struct pair *pair_data;
        struct env *env_data;
        struct inst *inst_data;
    } data;
};

struct val_list {
    struct val car;
    struct val_list *cdr;
};

/* -- pair
 * Represents a cons pair of two vals - `car` and `cdr`.
 */

struct pair {
    struct val car;
    struct val cdr;
};

/* -- lambda
 * Represents a lambda with the following elements:
 * - `params` is a linked list of strings representing parameter names.
 * - `body` contains the body of the lambda in the form of a pointer
 *   to program memory.
 * - `env` contains the environment in which the lambda is to be executed.
 *   A value of NULL represents the global environment.
 * `new_ptr` is used only for garbage collection and is normally NULL.
 */

struct env;

struct param_list {
    char *car;
    struct param_list *cdr;
};

struct lambda {
    struct param_list *params;
    struct inst *body;
    struct env *env;
    struct lambda *new_ptr;
};

/* -- binding
 * Represents a binding with the following elements:
 * - `val` is the value of the binding.
 * - `var` is the name of the bound variable.
 */

struct binding {
    struct val val;
    char *var;
};

/* -- env
 * Represents an environment with the following elements:
 * - `frame` contains a linked list of bindings.
 * - `outer` contains the outer environment frame.
 * `new_ptr` is used only for garbage collection and is normally NULL.
 * Environments are represented with pointers to `struct env`s,
 * where NULL is used to represent the global environment.
 */

struct frame {
    struct binding binding;
    struct frame *next;
};

struct env {
    struct frame *frame;
    struct env *outer;
    struct env *new_ptr;
};

/* -- prim_binding, high_prim_binding
 * Represents a binding to a primitive function.
 * Used in setting up the initial global environment.
 */

struct prim_binding {
    char *var;
    struct val (*val)(struct val *, int);
};

struct high_prim_binding {
    char *var;
    struct inst *(*val)(int);
};

/* -- cell
 * Represents a cell of garbage-collected memory on the heap.
 * May contain one of:
 * - `pair` - a pair
 * - `lambda` - a lambda
 * - `frame` - an environment frame
 * - `env` - an environment
 * The type of data contained in the cell must be represented by the context
 * of the pointer used to access it.
 */

union cell {
    struct pair pair;
    struct lambda lambda;
    struct frame frame;
    struct env env;
};

/* -- sexpr
 * Represents an expression in the program's AST before analysis.
 * It may take on the following values:
 * - Literal - SEXPR_LITERAL / literal
 * - Atom - SEXPR_ATOM / text
 * - Compound expression - SEXPR_CONS / cons
 */

enum sexpr_types {SEXPR_LITERAL, SEXPR_ATOM, SEXPR_CONS};

struct sexpr_list;

struct sexpr {
    enum sexpr_types type;
    union {
        struct val literal;
        char *atom;
        struct sexpr_list *cons;
    } data;
};

struct sexpr_list {
    struct sexpr *car;
    struct sexpr_list *cdr;
};

/* -- expr
 * Represents an expression in the program's AST.
 * It may take on the following values with the described fields:
 * - Literal - EXPR_LITERAL / literal
 * - Variable name - EXPR_VAR / var
 * - Function application - EXPR_APPL / appl
 * - Definition - EXPR_DEF / binding
 * - Assignment - EXPR_SET / binding
 * - Conditional expression - EXPR_IF / if_data
 *   * Note: The `alter` pointer may be NULL to indicate lack of alternative.
 * - Lambda expression - EXPR_LAMBDA / lambda
 * - Begin expression - EXPR_BEGIN / begin
 * - Quote expression - EXPR_QUOTE / quote
 */

enum expr_types {EXPR_LITERAL, EXPR_VAR, EXPR_APPL, EXPR_DEF, EXPR_SET,
    EXPR_IF, EXPR_LAMBDA, EXPR_BEGIN, EXPR_QUOTE};

struct expr_list;

struct expr {
    enum expr_types type;
    union {
        struct val literal;
        char *var;
        struct {
            struct expr *proc;
            struct expr_list *args;
        } appl;
        struct {
            char *var;
            struct expr *val;
        } binding;
        struct {
            struct expr *pred;
            struct expr *conseq;
            struct expr *alter;
        } if_data;
        struct {
            struct param_list *params;
            struct expr_list *body;
        } lambda;
        struct expr_list *begin;
        struct sexpr *quote;
    } data;
};

struct expr_list {
    struct expr *car;
    struct expr_list *cdr;
};

/* -- inst
 * Represents a virtual machine instruction.
 * The following are valid instructions:
 * - INST_CONST / val - Pushes a constant value onto the stack.
 * - INST_VAR / name - Looks up a variable in the current environment
 *   and pushes it onto the stack
 * - INST_DEF / name - Pops a value off the stack and binds it to `name`
 *   in the current environment frame.
 * - INST_SET / name - Pops a value off the stack and sets the value of `name`
 *   to it in the environment.
 * - INST_JUMP / ptr - Jumps to the instruction located at `ptr`.
 * - INST_JUMP_FALSE / ptr - Pops a value off the stack and jumps to `ptr`
 *   if it's false.
 * - INST_LAMBDA / lambda - Pushes a lambda with params `params`, body `ptr`,
 *   and env set to the current environment.
 * - INST_CALL / num - Calls the function with `num` arguments.
 *   The function and arguments are pushed in left-to-right order.
 *   Saves the environment and instruction pointer on the stack.
 * - INST_TAIL_CALL / num - Call a function with `num` arguments tail-recursively.
 * - INST_RETURN / -unspecified- - Returns from the current function.
 *   If there is not calling function, gives the return value as the final
 *   result of execution.
 * - INST_DELETE / -unspecified- - Pops a value off the stack without using it.
 * - INST_CONS / -unspecified- - Pops two values off the stack and pushes
 *   a pair containing the two of them.
 *   It's used in evaluating quoted literals instead of a function call,
 *   since cons can be rebound.
 */

enum inst_type {INST_CONST, INST_VAR, INST_DEF, INST_SET,
    INST_JUMP, INST_JUMP_FALSE, INST_LAMBDA,
    INST_CALL, INST_TAIL_CALL, INST_RETURN, INST_DELETE, INST_CONS};

struct inst {
    enum inst_type type;
    union {
        struct val val;
        char *name;
        struct inst *ptr;
        int num;
        struct {
            struct param_list *params;
            struct inst *ptr;
        } lambda;
    } args;
};
