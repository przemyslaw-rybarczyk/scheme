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
 * - Lambda - TYPE_LAMBDA / lambda_data
 * - Pair - TYPE_PAIR / pair_data
 * - Nil - TYPE_NIL / -unspecified-
 * - Void - TYPE_VOID / -unspecified-
 * Lambdas and pairs are represented as pointers to the heap.
 * Strings and symbols are immutable and may only be defined as literals.
 * A void type is used as a return value when none is specified, such as
 * in assignments and definitions.
 * A `type` value of TYPE_BROKEN_HEART is used only for garbage collection.
 */

enum types {TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_STRING, TYPE_SYMBOL,
    TYPE_PRIM, TYPE_LAMBDA, TYPE_PAIR, TYPE_NIL, TYPE_VOID, TYPE_BROKEN_HEART};

struct pair;
struct lambda;
struct val_list;

struct val {
    enum types type;
    union {
        long long int_data;
        double float_data;
        char *string_data;
        struct val (*prim_data)(struct val_list *);
        struct lambda *lambda_data;
        struct pair *pair_data;
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
 * - `body` contains the body of the lambda in the form of a linked list
 *   of expressions. The expressions are part of the program's AST.
 * - `env` contains the environment in which the lambda is to be executed.
 *   A value of null represents the global environment.
 * `new_ptr` is used only for garbage collection and is normally NULL.
 */

struct param_list {
    char *car;
    struct param_list *cdr;
};

struct lambda {
    struct param_list *params;
    struct expr_list *body;
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
 * - And expression - EXPR_AND / begin
 * - Or expression - EXPR_OR / begin
 */

enum expr_types {EXPR_LITERAL, EXPR_VAR, EXPR_APPL, EXPR_DEF, EXPR_SET,
    EXPR_IF, EXPR_LAMBDA, EXPR_BEGIN, EXPR_AND, EXPR_OR};

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
    } data;
};

struct expr_list {
    struct expr *car;
    struct expr_list *cdr;
};
