#pragma once

#include <stdint.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

struct Pair;
struct Lambda;
struct Global_env;

/* -- High_prim
 * Unlike a regular primitive function, a high primitive function takes
 * is called with the arguments still on top of the stack and returns
 * the program counter and global environment from which exec() should resume
 * execution. This is done to allow for primitive functions that run Scheme
 * code to be tail recursive.
 */

typedef struct High_prim_return {
    uint32_t pc;
    struct Global_env *global_env;
} High_prim_return;

typedef High_prim_return (*High_prim)(uint32_t);

/* -- Val
 * Contains a Scheme value with `type` representing its type and data
 * containing the value.
 * The valid representations are:
 * - Integer - TYPE_INT / int_data
 * - Floating-point - TYPE_FLOAT / float_data
 * - Boolean - TYPE_BOOL / int_data
 * - String - TYPE_STRING / string_data
 * - Symbol - TYPE_SYMBOL / string_data
 * - Primitive - TYPE_PRIM / prim_data
 * - High primitive - TYPE_HIGH_PRIM / prim_data
 * - Lambda - TYPE_LAMBDA / lambda_data
 * - Pair - TYPE_PAIR / pair_data
 * - Nil - TYPE_NIL / -unspecified-
 * - Void - TYPE_VOID / -unspecified-
 * - Undef - TYPE_UNDEF / -unspecified-
 * Lambdas and pairs are represented as pointers to the heap.
 * Strings and symbols are immutable and may only be defined as literals.
 * #!void is used as a return value when none is specified, such as
 * in assignments and definitions.
 * #!undef is used primarily for defining the letrec construct.
 * Attempting to read a variable whose value is #!undef results in an error.
 * A `type` value of TYPE_BROKEN_HEART is used only for garbage collection.
 * The following values are used only for returning from functions:
 * - Environment - TYPE_ENV / env_data
 * - Instruction pointer - TYPE_INST / inst_data
 * - Global environment - TYPE_GLOBAL_ENV / global_env_data
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
    TYPE_UNDEF,
    TYPE_BROKEN_HEART,
    TYPE_ENV,
    TYPE_INST,
    TYPE_GLOBAL_ENV,
} Type;

typedef struct Val {
    enum Type type;
    union {
        long long int_data;
        double float_data;
        char *string_data;
        struct Val (*prim_data)(struct Val *, uint32_t);
        High_prim high_prim_data;
        struct Lambda *lambda_data;
        struct Pair *pair_data;
        struct Env *env_data;
        struct Global_env *global_env_data;
        uint32_t inst_data;
    };
} Val;

/* -- Pair
 * Represents a cons pair of two vals - `car` and `cdr`.
 */

typedef struct Pair {
    Val car;
    Val cdr;
} Pair;

struct Env;

/* -- Lambda
 * Represents a lambda with the following elements:
 * - `params` is the number of parameters. If the highest bit is set, the function
 *   is variadic, with the remaining bits indicating the minimal number of arguments.
 * - `body` contains the body of the lambda in the form of an instruction address.
 * - `env` contains the environment in which the lambda is to be executed.
 * `new_ptr` is used only for garbage collection and is normally unused.
 */

#define PARAMS_VARIADIC 0x80000000

typedef struct Lambda {
    uint32_t params;
    uint32_t body;
    union {
        struct Env *env;
        struct Lambda *new_ptr;
    };
} Lambda;

typedef struct Binding {
    struct Val val;
    char *var;
} Binding;

typedef struct Global_env {
    uint32_t size;
    uint32_t capacity;
    struct Binding *bindings;
} Global_env;

/* -- Env
 * Represents an environment with the following elements:
 * - `outer` contains the outer environment.
 * - `size` contains the number of values in `vals`;
 * - `vals` contains the values bound in the lowest frame.
 * Environments are represented with pointers to `Env`s,
 * where NULL is used to represent the global environment.
 */

typedef struct Env {
    struct Env *outer;
    uint32_t size;
    struct Val vals[];
} Env;

/* -- env_loc
 * Represents the location of a variable in the environment.
 * `frame` equal to UINT32_MAX indicates the global environment.
 */

typedef struct Env_loc {
    uint32_t frame;
    uint32_t index;
} Env_loc;

/* -- inst
 * Represents a bytecode instruction.
 * The following are valid instructions:
 * - INST_CONST / val - Pushes a constant value onto the stack.
 * - INST_VAR / var - Finds a variable at the given location
 *   in the current environment and pushes it onto the stack.
 * - INST_NAME / name - Locates a name in the global environment
 *   and changes to a INST_VAR instruction without moving the program counter.
 * - INST_DEF / name - Pops a value off the stack and binds it to `name`
 *   in the global environment.
 * - INST_SET / var - Pops a value off the stack and assigns it
 *   to the variable located at `var` in the current environment.
 * - INST_SET_NAME / name - Locates a name in the global environment
 *   and changes to a INST_SET instruction without moving the program counter.
 * - INST_JUMP / index - Jumps to the instruction at given index.
 * - INST_JUMP_FALSE / index - Pops a value off the stack and jumps to
 *   the given index if it's false.
 * - INST_LAMBDA / lambda - Pushes a lambda with params `params`, body `index`,
 *   and env set to the current environment.
 * - INST_CALL / num - Calls the function with `num` arguments.
 *   The function and arguments are pushed in left-to-right order.
 *   Saves the environment and instruction pointer on the stack.
 * - INST_TAIL_CALL / num - Call a function with `num` arguments tail-recursively,
 *   without saving the environment and instruction pointer.
 * - INST_RETURN / -unspecified- - Returns from the current function.
 *   If there is no calling function, gives the return value as the final
 *   result of execution.
 * - INST_DELETE / -unspecified- - Pops a value off the stack without using it.
 * - INST_CONS / -unspecified- - Pops two values off the stack and pushes
 *   a pair containing the two of them.
 *   It's used in evaluating quoted literals instead of a function call,
 *   since cons can be rebound.
 * - INST_EXPR / -unspecified- - Indicates the beginning of a top-level expression.
 *   Does nothing when executed.
 * - INST_EOF / -unspecified- - Inserted at the end of code when it is loaded
 *   from a bytecode file.
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
        uint32_t index;
        uint32_t num;
        struct {
            uint32_t params;
            uint32_t index;
        } lambda;
    };
} Inst;

inline int is_true(Val val) {
    return !(val.type == TYPE_BOOL && val.int_data == 0);
}
