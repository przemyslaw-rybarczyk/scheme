#pragma once

#include <complex.h>
#include <inttypes.h>
#include <uchar.h>
#include "bigint/bigint.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

struct Fraction;
struct Complex;
struct Pair;
struct String;
struct Vector;
struct Lambda;
struct Global_env;
struct High_prim_return;
struct Val;

typedef struct High_prim_return High_prim(struct Val *, uint32_t);

/* -- Print_control
 * These values are put on the stack to control the process of printing a value.
 * - PRINT_CONTROL_END ends the printing function.
 * - PRINT_CONTROL_CDR interprets the next value on the stack as some suffix
 *   of a list and prints it accordingly depending on whether it is a pair, nil,
 *   or another value.
 * - PRINT_CONTROL_SPACE prints a space.
 * - PRINT_CONTROL_END_LIST prints a closed parenthesis.
 */
typedef enum Print_control {
    PRINT_CONTROL_END,
    PRINT_CONTROL_CDR,
    PRINT_CONTROL_SPACE,
    PRINT_CONTROL_END_LIST,
} Print_control;

/* -- Val
 * Contains a Scheme value with `type` representing its type and data
 * containing the value.
 * The valid representations are:
 * - Small integer - TYPE_INT / int_data
 * - Big integer - TYPE_BIGINT / bigint_data
 * - Fraction - TYPE_FRACTION / fraction_data
 * - Floating-point number - TYPE_FLOAT / float_data
 * - Complex floating-point number - TYPE_FLOAT_COMPLEX / float_complex_data
 * - Complex number - TYPE_COMPLEX / complex_data
 * - Boolean - TYPE_BOOL / int_data
 * - String - TYPE_STRING / string_data
 * - Symbol - TYPE_SYMBOL / string_data
 * - Primitive - TYPE_PRIM / prim_data
 * - High primitive - TYPE_HIGH_PRIM / prim_data
 * - Lambda - TYPE_LAMBDA / lambda_data
 * - Pair - TYPE_PAIR / pair_data
 * - Vector - TYPE_VECTOR / vector_data
 * - Nil - TYPE_NIL / -unspecified-
 * - Void - TYPE_VOID / -unspecified-
 * - Undef - TYPE_UNDEF / -unspecified-
 *
 * Some types have constant variants, which are prefixed with with TYPE_CONST.
 * They represent literal values and as a result are immutable and not subject
 * to garbage collection.
 *
 * The six numeric types form a kind of hierarchy where values of a lower type
 * could also be represented using a higher type:
 * ┌───────────────────────────────────────────────┐
 * │               TYPE_COMPLEX                    │
 * │              ↙     ↓      ↘                   │
 * │ TYPE_FRACTION  TYPE_FLOAT  TYPE_FLOAT_COMPLEX │
 * │      ↓                                        │
 * │ TYPE_BIGINT                                   │
 * │      ↓                                        │
 * │ TYPE_INT                                      │
 * └───────────────────────────────────────────────┘
 * However, a numeric value must be represented in the simplest form possible. As such:
 * - Integers in range [SMALL_INT_MIN, SMALL_INT_MAX] must be represented as TYPE_INT.
 * - Fractions with denominator 1 must be represented as integers.
 * - Other fractions must be fully reduced.
 * - Complex numbers whose imaginary part is an exact 0 must be represented with
 *   an appropriate real type.
 * - Complex numbers whose both parts are inexact must be represented as TYPE_FLOAT_COMPLEX.
 * These rules ensure that each number has exactly one valid representation.
 *
 * #!void is used as a return value when none is specified, such as
 * in assignments and definitions.
 * #!undef is used primarily for defining the letrec construct.
 * Attempting to read a variable whose value is #!undef results in an error.
 * However, for simplicity, this property is disabled in the compiler.
 *
 * A `type` value of TYPE_BROKEN_HEART is used internally to mark an invalid value,
 * primarily in garbage collection.
 * The following values are used only for returning from functions:
 * - Environment - TYPE_ENV / env_data
 * - Instruction pointer - TYPE_INST / inst_data
 * - Global environment - TYPE_GLOBAL_ENV / global_env_data
 * TYPE_PRINT_CONTROL is used only within functions used to print variables.
 */

typedef enum Type {
    TYPE_INT,
    TYPE_BIGINT,
    TYPE_CONST_BIGINT,
    TYPE_FRACTION,
    TYPE_CONST_FRACTION,
    TYPE_FLOAT,
    TYPE_FLOAT_COMPLEX,
    TYPE_CONST_FLOAT_COMPLEX,
    TYPE_COMPLEX,
    TYPE_CONST_COMPLEX,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_CONST_STRING,
    TYPE_SYMBOL,
    TYPE_PRIM,
    TYPE_HIGH_PRIM,
    TYPE_LAMBDA,
    TYPE_PAIR,
    TYPE_CONST_PAIR,
    TYPE_VECTOR,
    TYPE_CONST_VECTOR,
    TYPE_NIL,
    TYPE_VOID,
    TYPE_UNDEF,
    // ↓ internal use only
    TYPE_BROKEN_HEART,
    TYPE_ENV,
    TYPE_INST,
    TYPE_GLOBAL_ENV,
    TYPE_PRINT_CONTROL,
} Type;

typedef struct Val {
    enum Type type;
    union {
        small_int int_data;
        Bigint *bigint_data;
        struct Fraction *fraction_data;
        double float_data;
        double complex *float_complex_data;
        struct Complex *complex_data;
        char32_t char_data;
        struct String *string_data;
        struct Val (*prim_data)(struct Val *, uint32_t);
        High_prim *high_prim_data;
        struct Lambda *lambda_data;
        struct Pair *pair_data;
        struct Vector *vector_data;
        struct Env *env_data;
        struct Global_env *global_env_data;
        uint32_t inst_data;
        enum Print_control print_control_data;
    };
} Val;

typedef struct Fraction {
    Bigint *numerator;
    Bigint *denominator;
} Fraction;

// Each value must be of a real number type.
typedef struct Complex {
    Val real;
    Val imag;
} Complex;

typedef struct Pair {
    Val car;
    Val cdr;
} Pair;

struct Env;

/* -- String
 * A string may be allocated in one of three ways. Uninterned strings are used
 * to represent string constants. Interned strings are used to represent symbols
 * and variable names. GC-allocated strings represent mutable strings and are
 * allocated from garbage-collected memory.
 * Every GC-allocated string must have at least one character, even if it's empty.
 * The character is used for purposes of garbage collection.
 */
typedef struct String {
    union {
        size_t len;
        struct String *new_ptr;
    };
    char32_t chars[];
} String;

/* -- Vector
 * A vector may be constant or allocated in garbage-collected memory.
 * Every GC-allocated vector must hold at least one value, even if it's empty.
 * The value is used for garbage collection.
 */
typedef struct Vector {
    union {
        size_t len;
        struct Vector *new_ptr;
    };
    struct Val vals[];
} Vector;

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
    String *var;
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

/* -- High_prim
 * Unlike a regular primitive function, a high primitive function
 * is called with the arguments still on top of the stack and returns
 * the program counter and global environment from which exec() should resume
 * execution. This is done to allow for primitive functions that run Scheme
 * code to be tail recursive.
 */

typedef struct High_prim_return {
    uint32_t pc;
    struct Global_env *global_env;
} High_prim_return;

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
        Val val;
        struct Env_loc var;
        String *name;
        uint32_t index;
        uint32_t num;
        struct {
            uint32_t params;
            uint32_t index;
        } lambda;
    };
} Inst;
