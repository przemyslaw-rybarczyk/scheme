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

typedef enum Type {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_SYMBOL,
    TYPE_PRIM,
    TYPE_HIGH_PRIM,
    TYPE_LAMBDA,
    TYPE_PAIR,
    TYPE_NIL,
    TYPE_VOID,
    TYPE_BROKEN_HEART,
    TYPE_ENV,
    TYPE_INST
} Type;

struct Pair;
struct Lambda;

typedef struct Val {
    enum Type type;
    union {
        long long int_data;
        double float_data;
        char *string_data;
        struct Val (*prim_data)(struct Val *, int);
        int (*high_prim_data)(int);
        struct Lambda *lambda_data;
        struct Pair *pair_data;
        struct Env *env_data;
        int inst_data;
    };
} Val;

/* -- pair
 * Represents a cons pair of two vals - `car` and `cdr`.
 */

typedef struct Pair {
    Val car;
    Val cdr;
} Pair;

/* -- lambda
 * Represents a lambda with the following elements:
 * - `params` is a linked list of strings representing parameter names.
 * - `body` contains the body of the lambda in the form of a pointer
 *   to program memory.
 * - `env` contains the environment in which the lambda is to be executed.
 *   A value of NULL represents the global environment.
 * `new_ptr` is used only for garbage collection and is normally NULL.
 */

struct Env;

// TODO remove this type
struct name_list {
    char *car;
    struct name_list *cdr;
};

typedef struct Lambda {
    int params;
    int body;
    struct Env *env;
    struct Lambda *new_ptr;
} Lambda;

/* -- binding
 * Represents a binding with the following elements:
 * - `val` is the value of the binding.
 * - `var` is the name of the bound variable.
 */

typedef struct Binding {
    struct Val val;
    char *var;
} Binding;

/* -- env
 * Represents an environment with the following elements:
 * - `frame` contains a linked list of bindings.
 * - `outer` contains the outer environment frame.
 * `new_ptr` is used only for garbage collection and is normally NULL.
 * Environments are represented with pointers to `Env`s,
 * where NULL is used to represent the global environment.
 * TODO describe
 */

typedef struct Env {
    struct Env *outer;
    int size;
    struct Val vals[];
} Env;

/* -- prim_binding, high_prim_binding
 * Represents a binding to a primitive function.
 * Used in setting up the initial global environment.
 */

struct prim_binding {
    char *var;
    struct Val (*val)(Val *, int);
};

struct high_prim_binding {
    char *var;
    int (*val)(int);
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
        Val literal;
        char *atom;
        struct sexpr_list *cons;
    };
};

struct sexpr_list {
    struct sexpr *car;
    struct sexpr_list *cdr;
};

/* -- name_env
 */

struct name_env {
    struct name_list *frame;
    struct name_env *next;
};

/* -- env_loc
 * TODO explain
 */

typedef struct Env_loc {
    int frame;
    int index;
} Env_loc;

/* -- expr
 * Represents an expression in the program's AST.
 * It may take on the following values with the described fields:
 * - Literal - EXPR_LITERAL / literal
 * - Variable location - EXPR_VAR / var
 * - Global variable name - EXPR_NAME / name
 * - Function application - EXPR_APPL / appl
 * - Definition - EXPR_DEF / binding
 * - Assignment - EXPR_SET / binding
 * - Conditional expression - EXPR_IF / if_data
 *   * Note: The `alter` pointer may be NULL to indicate lack of alternative.
 * - Lambda expression - EXPR_LAMBDA / lambda
 * - Begin expression - EXPR_BEGIN / begin
 * - Quote expression - EXPR_QUOTE / quote
 */

enum expr_types {EXPR_LITERAL, EXPR_VAR, EXPR_NAME, EXPR_APPL,
    EXPR_DEF, EXPR_SET, EXPR_SET_NAME, EXPR_IF, EXPR_LAMBDA, EXPR_BEGIN, EXPR_QUOTE};

struct expr_list;

struct expr {
    enum expr_types type;
    union {
        Val literal;
        Env_loc var;
        char *name;
        struct {
            struct expr *proc;
            struct expr_list *args;
        } appl;
        struct {
            Env_loc var;
            struct expr *val;
        } binding;
        struct {
            char *var;
            struct expr *val;
        } name_binding;
        struct {
            struct expr *pred;
            struct expr *conseq;
            struct expr *alter;
        } if_data;
        struct {
            int params;
            struct expr_list *body;
        } lambda;
        struct expr_list *begin;
        struct sexpr *quote;
    };
};

struct expr_list {
    struct expr *car;
    struct expr_list *cdr;
};

/* -- inst
 * Represents a virtual machine instruction.
 * The following are valid instructions:
 * - INST_CONST / val - Pushes a constant value onto the stack.
 * - INST_VAR / name - Looks up a variable with a given name
 *   in the current environment and pushes it onto the stack.
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
 * TODO update
 */

enum Inst_type {INST_CONST, INST_VAR, INST_NAME, INST_DEF,
    INST_SET, INST_SET_NAME, INST_JUMP, INST_JUMP_FALSE, INST_LAMBDA,
    INST_CALL, INST_TAIL_CALL, INST_RETURN, INST_DELETE, INST_CONS,
    INST_EXPR, INST_EOF};

typedef struct Inst {
    enum Inst_type type;
    union {
        struct Val val;
        struct Env_loc var;
        char *name;
        int index;
        int num;
        struct {
            int params;
            int index;
        } lambda;
    };
} Inst;
